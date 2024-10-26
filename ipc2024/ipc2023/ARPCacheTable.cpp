// ARPCacheTable.cpp
#include "ARPCacheTable.h"
#include "pch.h"

ARPCacheTable::ARPCacheTable() {
    // 필요한 초기화 작업을 수행할 수 있습니다.
}

ARPCacheTable::~ARPCacheTable() {
    for (auto& pair : cache) {
        delete pair.second;
    }
}

std::string ARPCacheTable::binaryToString(const unsigned char* ip) {
    return std::to_string(ip[0]) + "." +
        std::to_string(ip[1]) + "." +
        std::to_string(ip[2]) + "." +
        std::to_string(ip[3]);
}

bool ARPCacheTable::addOrPresent(const unsigned char* ip, const unsigned char* mac, const bool& state, const bool& isPermanent) {
    std::string strIP = binaryToString(ip);
    auto it = cache.find(strIP);
    if (it != cache.end()) {
        return false;
    }
    else {
        cache[strIP] = new ARPCacheEntry(ip, mac, state, this, isPermanent);
        return true;
    }
}

void ARPCacheTable::handleArpReply(const unsigned char* ip) {
    std::string strIP = binaryToString(ip);
    auto it = cache.find(strIP);
    if (it != cache.end()) {
        it->second->onArpReplyReceived();
    }
}

void ARPCacheTable::removeEntry(const unsigned char* ip) {
    std::string strIP = binaryToString(ip);
    auto it = cache.find(strIP);
    if (it != cache.end()) {
        delete it->second;
        cache.erase(it);
    }
}

bool ARPCacheTable::clearAll() {
    if (cache.empty()) {
        return false; // cache가 이미 비어 있는 경우 false 반환
    }
    try {
        for (auto& pair : cache) {
            delete pair.second;  // ARPCacheEntry 객체의 메모리 해제
        }
        cache.clear();  // 맵 초기화
    }
    catch (const std::exception& e) {
        return false;  // 메모리 접근 실패 시 false 반환
    }
    cache.clear();
    return true;
}

//void ARPCacheTable::printCache() const {
//    for (const auto& pair : cache) {
//        const auto& ip = pair.first;
//        const auto& entry = pair.second;
//        std::cout << "IP: " << entry->getIP() << ", MAC: " << entry->getMAC() << ", State: " << entry->getState() << std::endl;
//    }
//}
