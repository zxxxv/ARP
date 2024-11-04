#include "pch.h"
#include "ARPCacheTable.h"

ARPCacheTable::ARPCacheTable() {
    // 필요한 초기화 작업을 수행할 수 있습니다.
    std::cout << "ARPCacheTable initialized." << std::endl;
}

ARPCacheTable::~ARPCacheTable() {
    clearAll();  // 모든 캐시 항목을 안전하게 삭제
}

std::string ARPCacheTable::binaryToString(const unsigned char* ip) {
    return std::to_string(ip[0]) + "." +
        std::to_string(ip[1]) + "." +
        std::to_string(ip[2]) + "." +
        std::to_string(ip[3]);
}

bool ARPCacheTable::addOrUpdate(const unsigned char* ip, const unsigned char* mac, const bool& state, const bool& isPermanent) {
    std::string strIP = binaryToString(ip);
    auto it = cache.find(strIP);
    if (it != cache.end()) {
        editEntryMacAddress(ip, mac);
        return false;
    }
    else {
        cache[strIP] = new ARPCacheEntry(ip, mac, state, this, isPermanent);
        return true;
    }
}

bool ARPCacheTable::editEntryMacAddress(const unsigned char* ip, const unsigned char* mac) {
    if (!ip || !mac) return false;
    std::string strIP = binaryToString(ip);
    auto it = cache.find(strIP);
    if (it != cache.end()) {
        it->second->editMac(mac);
        return true;
    }
    return false;
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
        return false;
    }
    try {
        for (auto& pair : cache) {
            delete pair.second;
        }
        cache.clear();
    }
    catch (const std::exception& e) {
        return false;
    }
    return true;
}

void ARPCacheTable::printCache() const {
    CString accumulatedMessage;
    for (const auto& pair : cache) {
        const auto& ip = pair.first;
        const auto& entry = pair.second;

        CString CIp(entry->getIP().c_str());
        CString CMac(entry->getMAC().c_str());
        CString CState(entry->getState().c_str());

        CString line;
        line.Format(_T("IP: %s, MAC: %s, State: %s\r\n"), CIp, CMac, CState);
        accumulatedMessage += line;
    }
    AfxMessageBox(accumulatedMessage);
}
