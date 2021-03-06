/*
 * KSockPoolHelper.h
 *
 *  Created on: 2010-6-4
 *      Author: keengo
 */

#ifndef KSOCKPOOLHELPER_H_
#define KSOCKPOOLHELPER_H_
/*
 * Copyright (c) 2010, NanChang BangTeng Inc
 * All Rights Reserved.
 *
 * You may use the Software for free for non-commercial use
 * under the License Restrictions.
 *
 * You may modify the source code(if being provieded) or interface
 * of the Software under the License Restrictions.
 *
 * You may use the Software for commercial use after purchasing the
 * commercial license.Moreover, according to the license you purchased
 * you may get specified term, manner and content of technical
 * support from NanChang BangTeng Inc
 *
 * See COPYING file for detail.
 */
#include <map>
#include <string>
#include <list>
#include "KSocket.h"
#include "global.h"
#include "KCountable.h"
#include "KPoolableSocketContainer.h"
#include "KHttpRequest.h"
#define ERROR_RECONNECT_TIME	600
/**
* 服务器扩展节点
*/
class KSockPoolHelper : public KPoolableSocketContainer {
public:
	KSockPoolHelper();
	virtual ~KSockPoolHelper();
	void connect(KHttpRequest *rq);
	KUpstreamSelectable *getConnection(KHttpRequest *rq,bool &half,bool &need_name_resolved);
	KUpstreamSelectable *newConnection(KHttpRequest *rq, bool &need_name_resolved);
	void isBad(KUpstreamSelectable *st,BadStage stage)
	{
		switch(stage){
		case BadStage_Connect:
		case BadStage_TrySend:
			error_count++;
			if (error_count>=max_error_count) {
				disable();
			}
			break;
		default:
			break;
		}		
	}
	bool isChanged(KSockPoolHelper *sh)
	{
		if (host!=sh->host) {
			return true;
		}
		if (port!=sh->port) {
			return true;
		}
#ifdef ENABLE_UPSTREAM_SSL
		if (ssl!=sh->ssl) {
			return true;
		}
#endif
		if (weight!=sh->weight) {
			return true;
		}
		if (isUnix!=sh->isUnix) {
			return true;
		}
		if (isIp!=sh->isIp) {
			return true;
		}
		if (ip!=sh->ip) {
			return true;
		}
		if (ip && strcmp(ip,sh->ip)!=0) {
			return true;
		}
		if (lifeTime != sh->lifeTime) {
			return true;
		}
		return false;
	}
	void isGood(KUpstreamSelectable *st)
	{
		enable();
	}
	void start_monitor_call_back();
	void syncCheckConnect();
	void setErrorTryTime(int max_error_count, int error_try_time)
	{
		lock.Lock();
		this->max_error_count = max_error_count;
		this->error_try_time = error_try_time;
		if (max_error_count > 0) {
			internalStopMonitor();
		} else {
			startMonitor();
		}
		lock.Unlock();
	}
	void checkActive();
	bool setHostPort(std::string host,int port,const char *ssl);
	bool setHostPort(std::string host, const char *port);
	void disable();
	void enable();
	bool isEnable();
	void setIp(const char *ip)
	{
		if (this->ip) {
			free(this->ip);
			this->ip = NULL;
		}
		if (ip && *ip) {
			this->ip = strdup(ip);
		}
	}
	const char *getIp()
	{
		return this->ip;
	}
	int hit;
	int weight;
	std::string host;	
	int port;
	bool isUnix;
	bool isIp;
	/////////[261]
	int error_try_time;
	/*
	 * 连续错误连接次数，如果超过MAX_ERROR_COUNT次，就会认为是问题的。
	 * 下次试连接时间会从当前时间加ERROR_RECONNECT_TIME秒。
	 */
	int error_count;
	int max_error_count;
	/*
	 * 下次试连接时间，如果是0表示活跃的。
	 */
	time_t tryTime;
	bool real_connect(KHttpRequest *rq,KUpstreamSelectable *socket);
	void buildXML(std::stringstream &s);
	bool parse(std::map<std::string,std::string> &attr);
	void monitorConnectStage(KHttpRequest *rq, KUpstreamSelectable *socket);
	void monitorNextTick();
	void stopMonitor()
	{
		lock.Lock();
		internalStopMonitor();
		lock.Unlock();
	}
	KSockPoolHelper *next;
	KSockPoolHelper *prev;
private:	
	void startMonitor();
	void internalStopMonitor()
	{
		monitor = false;
	}
	char *ip;
	bool auto_detected_ip;
	bool monitor;
	KMutex lock;
/////////[262]
};

#endif /* KSOCKPOOLHELPER_H_ */
