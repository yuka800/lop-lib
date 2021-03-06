/* RedLock implement DLM(Distributed Lock Manager) with cpp.
 *
 * Copyright (c) 2014, jacketzhong <jacketzhong at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <BCLib/utility/dateTime.h>

#if defined(_LINUX)
#include <unistd.h>
#include "unistd.h"
#else
#include "windows.h"
#endif

#include <math.h>
#include <MWLib/redis/redlock.h>
#include <MWLib/redis/redisClient.h>
 /* Turn the plain C strings into Sds strings */
static char **convertToSds(int count, char** args) {
    int j;
    char **sds = (char**)malloc(sizeof(char*)*count);
    for (j = 0; j < count; j++)
        sds[j] = sdsnew(args[j]);
    return sds;
}
namespace MWLib
{
	namespace Redis
	{
// ----------------
// init redlock
// ----------------
CRedLock::CRedLock() {
    Initialize();

}

// ----------------
// release redlock
// ----------------
CRedLock::~CRedLock() {
    sdsfree(m_continueLockScript);
    sdsfree(m_unlockScript);

}

// ----------------
// Initialize the server: scripts...
// ----------------
bool CRedLock::Initialize() {
    m_defaultRetryCount = 3;
    m_defaultRetryDelay = 200;
    m_clockDriftFactor = 0.01f;

    m_continueLockScript = sdsnew("if redis.call('get', KEYS[1]) == ARGV[1] then redis.call('del', KEYS[1]) end return redis.call('set', KEYS[1], ARGV[2], 'px', ARGV[3], 'nx')");
    m_unlockScript = sdsnew("if redis.call('get', KEYS[1]) == ARGV[1] then return redis.call('del', KEYS[1]) else return 0 end");
    m_retryCount = m_defaultRetryCount;
    m_retryDelay = m_defaultRetryDelay;
    m_quoRum = 0;
	m_pRedisClient = NULL;
    return true;
}

// ----------------
// add redis server
// ----------------
bool CRedLock::AddServerContext(redisContext * c, BCLib::uint16 type, CRedisClient *pRedisClient) {

    if (c != NULL) {
		m_redisServerMap[type] = c;
        //m_redisServer.push_back(c);
    }
	if (pRedisClient != NULL)
	{
		m_pRedisClient = pRedisClient;
	}
    m_quoRum = 1;
	//m_quoRum = (int)m_redisServer.size() / 2 + 1;
    return true;
}
bool CRedLock::delServerContext(redisContext * c, BCLib::uint16 type)
{
    if (c == NULL)
    {
        return true;
    }
    //std::vector<redisContext *>::iterator it = m_redisServer.begin();
	m_redisServerMap.erase(type);
    /*for (; it != m_redisServer.end(); it++)
    {
        if (*it == c)
        {
            it = m_redisServer.erase(it);
            break;
        }
    }*/
    return true;
}
// ----------------
// set retry
// ----------------
void CRedLock::SetRetry(const int count, const int delay) {
    m_retryCount = count;
    m_retryDelay = delay;
}

// ----------------
// lock the resource
// ----------------
int CRedLock::Lock(const char *resource, const int ttl, CLock &lock, BCLib::uint16 type)
{
    sds val = GetUniqueLockId();
    if (!val) {
        return -1;
    }
    lock.m_resource = sdsnew(resource);
    lock.m_val = val;
    //printf("Get the unique id is %s\n", val);
    int retryCount = m_retryCount;
    do {
        int n = 0;
        BCLib::Utility::CSystemTime now;
        BCLib::uint64 startTime = now.getMilliseconds();

        /*int slen = (int)m_redisServer.size();
        for (int i = 0; i < slen; i++) {
            if (LockInstance(m_redisServer[i], resource, val, ttl) == 0) {
                n++;
            }
        }*/
		std::unordered_map<BCLib::uint16, redisContext *>::iterator it = m_redisServerMap.find(type);
		int ret = 0;
		if (it != m_redisServerMap.end())
		{
			ret = LockInstance(it->second, resource, val, ttl);
			if (it->second != NULL && ret == 0) {
				n++;
			}
		}
		if (ret == -2)
		{
			printf("Lock error can not find redis context by type = %d !", type);
			if (it->second)
			{
				this->delServerContext(it->second, type);
				m_pRedisClient->disconnect((MWLib::Redis::EREDIS_CONTEXT_TYPE)type);
			}		
			return -2;
		}
		if (n <= 0)
		{
			printf("Lock error get lock faild, type = %d !", type);
			return -1;
		}
        //Add 2 milliseconds to the drift to account for Redis expires
        //precision, which is 1 millisecond, plus 1 millisecond min drift
        //for small TTLs.
        int drift = (int)(ttl * m_clockDriftFactor) + 2;
        BCLib::Utility::CSystemTime now1;
        BCLib::uint64 nowTime = now1.getMilliseconds();
        BCLib::uint64 validityTime = ttl - (nowTime - startTime) - drift;
        //printf("The resource validty time is %d, n is %d, quo is %d\n",
        //       validityTime, n, m_quoRum);
        if (n >= m_quoRum && validityTime > 0) {
            lock.m_validityTime = (int)validityTime;
            return 0;
        }
        else {
            Unlock(lock, type);
        }
        // Wait a random delay before to retry
        //int delay = rand() % m_retryDelay/10 + (int)floor(m_retryDelay / 20);

#if defined(_LINUX)
        //usleep(delay * 1000);
		usleep(1000);
#else
        //Sleep(delay);
		Sleep(1);
#endif
        retryCount--;
    } while (retryCount > 0);
    return -1;
}

// ----------------
// release resource
// ----------------
bool CRedLock::ContinueLock(const char *resource, const int ttl, CLock &lock, BCLib::uint16 type) {
    sds val = GetUniqueLockId();
    if (!val) {
        return false;
    }
    lock.m_resource = sdsnew(resource);
    lock.m_val = val;
    if (m_continueLock.m_resource == NULL) {
        m_continueLock.m_resource = sdsnew(resource);
        m_continueLock.m_val = sdsnew(val);
    }
    //printf("Get the unique id is %s\n", val);
    int retryCount = m_retryCount;
    do {
        int n = 0;
        int startTime = (int)time(NULL) * 1000;
        //int slen = (int)m_redisServer.size();
        //for (int i = 0; i < slen; i++) {
        //    if (ContinueLockInstance(m_redisServer[i], resource, val, ttl)) {
        //        n++;
        //    }
        //}
		std::unordered_map<BCLib::uint16, redisContext *>::iterator it = m_redisServerMap.find(type);
		if (it != m_redisServerMap.end() && it->second != NULL)
		{
			if (ContinueLockInstance(it->second, resource, val, ttl))
			{
				n++;
			}
		}
		if (n <= 0)
		{
			printf("ContinueLock error can not find redis context by type = %d !", type);
			return false;
		}
        // update old val
        sdsfree(m_continueLock.m_val);
        m_continueLock.m_val = sdsnew(val);
        //Add 2 milliseconds to the drift to account for Redis expires
        //precision, which is 1 millisecond, plus 1 millisecond min drift
        //for small TTLs.
        int drift = (int)(ttl * m_clockDriftFactor) + 2;
        int validityTime = ttl - ((int)time(NULL) * 1000 - startTime) - drift;
        //printf("The resource validty time is %d, n is %d, quo is %d\n",
        //       validityTime, n, m_quoRum);
        if (n >= m_quoRum && validityTime > 0) {
            lock.m_validityTime = validityTime;
            return true;
        }
        else {
            Unlock(lock, type);
        }
        // Wait a random delay before to retry
        //int delay = rand() % m_retryDelay + (int)floor(m_retryDelay / 2);
#if defined(_LINUX)
        //usleep(delay * 1000);
		usleep(100);
#else
        //Sleep(delay);
		Sleep(1);
#endif
        retryCount--;
    } while (retryCount > 0);
    return false;
}

// ----------------
// lock the resource
// ----------------
bool CRedLock::Unlock(const CLock &lock, BCLib::uint16 type)
{
    //int slen = (int)m_redisServer.size();
    //for (int i = 0; i < slen; i++) {
    //    UnlockInstance(m_redisServer[i], lock.m_resource, lock.m_val);
    //}
	std::unordered_map<BCLib::uint16, redisContext *>::iterator it = m_redisServerMap.find(type);
	if (it != m_redisServerMap.end() && it->second != NULL)
	{
		UnlockInstance(it->second, lock.m_resource, lock.m_val);
	}
    return true;
}

// ----------------
// lock the resource milliseconds
// ----------------
int CRedLock::LockInstance(redisContext *c, const char *resource, const char *val, const int ttl) 
{
    redisReply *reply;
    reply = (redisReply *)redisCommand(c, "set %s %s px %d nx",
        resource, val, ttl);
    if (reply) {
         printf("Set return: %s [null == fail, OK == success]\n", reply->str);
    }

	if (reply == NULL)
	{
		return -2;
	}

    if (reply && reply->str && strcmp(reply->str, "OK") == 0) {
        freeReplyObject(reply);
        return 0;
    }
    if (reply) {
        freeReplyObject(reply);
    }
	if (c)
	{
		c->obuf = sdsempty();
	}
    return -1;
}

// ----------------
// 对redismaster续锁, 私有函数
// ----------------
bool CRedLock::ContinueLockInstance(redisContext *c, const char *resource,
    const char *val, const int ttl) {
    int argc = 7;
    sds sdsTTL = sdsempty();
    sdsTTL = sdscatprintf(sdsTTL, "%d", ttl);
    char *continueLockScriptArgv[] = { (char*)"EVAL",
                                      m_continueLockScript,
                                      (char*)"1",
                                      (char*)resource,
                                      m_continueLock.m_val,
                                      (char*)val,
                                      sdsTTL };
    redisReply *reply = RedisCommandArgv(c, argc, continueLockScriptArgv);
    sdsfree(sdsTTL);
    if (reply) {
        printf("Set return: %s [null == fail, OK == success]\n", reply->str);
    }
    if (reply && reply->str && strcmp(reply->str, "OK") == 0) {
        freeReplyObject(reply);
        return true;
    }
    if (reply) {
        freeReplyObject(reply);
    }
    return false;
}

// ----------------
// 对redismaster解锁
// ----------------
void CRedLock::UnlockInstance(redisContext *c, const char *resource,
    const char *val) {
    int argc = 5;
    //printf("Redis unlock script = [%s]", m_unlockScript);
    char *unlockScriptArgv[] = { (char*)"EVAL",
                                m_unlockScript,
                                (char*)"1",
                                (char*)resource,
                                (char*)val };
    redisReply *reply = RedisCommandArgv(c, argc, unlockScriptArgv);
    if (reply) {
        freeReplyObject(reply);
    }
}

// ----------------
// 对redismaster执行脚本命令
// ----------------
redisReply * CRedLock::RedisCommandArgv(redisContext *c, int argc, char **inargv) {
    char **argv;
    argv = convertToSds(argc, inargv);
    /* Setup argument length */
    size_t *argvlen;
    argvlen = (size_t *)malloc(argc * sizeof(size_t));
    for (int j = 0; j < argc; j++)
        argvlen[j] = sdslen(argv[j]);
    redisReply *reply = NULL;
    reply = (redisReply *)redisCommandArgv(c, argc, (const char **)argv, argvlen);
    if (reply) {
        //printf("RedisCommandArgv return: %lld\n", reply->integer);
    }
    free(argvlen);
    sdsfreesplitres(argv, argc);
    return reply;
}

// ----------------
// 获取唯一id
// ----------------
sds CRedLock::GetUniqueLockId() {
    unsigned char buffer[20] = { 0 };
    sds s;
    s = sdsempty();
    unsigned int pid = 0;
#if defined(_LINUX)
    pid = getpid();
#else
    pid = GetCurrentProcessId();
#endif
    //printf("pid = %d", pid);
    srand((unsigned int)time(NULL) + pid);
    for (int i = 0; i < 20; i++)
    {
        buffer[i] = rand() % 26 + 'a';
    }

    s = sdsempty();
    for (int i = 0; i < 20; i++) {
        s = sdscatprintf(s, "%c", buffer[i]);
    }
    return s;
}
	}//Redis
}//MWLib