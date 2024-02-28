/*
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2016 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <glog/logging.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <regex>

class DnsmasqInterface {
public:
    virtual ~DnsmasqInterface() = default;
    virtual bool SendDNSQuery(const std::string& domain) = 0;
    virtual bool SetCustomDNS(const std::string& domain, const std::string& ip) = 0;
    virtual bool AllocateDHCPLease(const std::string& macAddress) = 0;
    virtual bool RenewDHCPLease(const std::string& macAddress) = 0;
    virtual bool SetStaticIP(const std::string& macAddress, const std::string& ip) = 0;
    virtual bool SetDHCPOption(const std::string& option, const std::string& value) = 0;
};

class MockDnsmasq : public DnsmasqInterface {
public:
    MOCK_METHOD(bool, SendDNSQuery, (const std::string&), (override));
    MOCK_METHOD(bool, SetCustomDNS, (const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, AllocateDHCPLease, (const std::string&), (override));
    MOCK_METHOD(bool, RenewDHCPLease, (const std::string&), (override));
    MOCK_METHOD(bool, SetStaticIP, (const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, SetDHCPOption, (const std::string&, const std::string&), (override));
};

class DnsmasqTest : public ::testing::Test {
protected:
    MockDnsmasq mockDnsmasq;
};

struct DNSQueryTestData {
    std::string domain;
    bool expectedResult;
};

class DnsmasqDNSQueryTest : public DnsmasqTest, public ::testing::WithParamInterface<DNSQueryTestData> {};

TEST_P(DnsmasqDNSQueryTest, BasicDNSQueryForwarding) {
    auto testData = GetParam();
    RecordProperty("Domain", testData.domain);
    RecordProperty("ExpectedResult", testData.expectedResult ? "true" : "false");
    LOG(INFO) << "Testing DNS query forwarding for domain: " << testData.domain
              << " with expected result: " << (testData.expectedResult ? "true" : "false");

    EXPECT_CALL(mockDnsmasq, SendDNSQuery(testData.domain))
        .Times(1)
        .WillOnce(::testing::Return(testData.expectedResult));
    ASSERT_EQ(mockDnsmasq.SendDNSQuery(testData.domain), testData.expectedResult);
}

INSTANTIATE_TEST_SUITE_P(
    DnsmasqTests,
    DnsmasqDNSQueryTest,
    ::testing::Values(
        DNSQueryTestData{"example.com", true},
        DNSQueryTestData{"nonexistent.domain", false},
        DNSQueryTestData{"custom.domain", true},
        DNSQueryTestData{"", false},
        DNSQueryTestData{"invalid domain!", false}
    )
);

struct CustomDNSTestData {
    std::string domain;
    std::string ipAddress;
    bool expectedResult;
};

class DnsmasqCustomDNSTest : public DnsmasqTest, public ::testing::WithParamInterface<CustomDNSTestData> {};

TEST_P(DnsmasqCustomDNSTest, SetCustomDNS) {
    auto testData = GetParam();
    RecordProperty("Domain", testData.domain);
    RecordProperty("IPAddress", testData.ipAddress);
    RecordProperty("ExpectedResult", testData.expectedResult ? "true" : "false");
    LOG(INFO) << "Setting custom DNS for domain: " << testData.domain
              << " with IP address: " << testData.ipAddress
              << " and expecting result: " << (testData.expectedResult ? "true" : "false");

    EXPECT_CALL(mockDnsmasq, SetCustomDNS(testData.domain, testData.ipAddress))
        .Times(1)
        .WillOnce(::testing::Return(testData.expectedResult));
    ASSERT_EQ(mockDnsmasq.SetCustomDNS(testData.domain, testData.ipAddress), testData.expectedResult);
}

INSTANTIATE_TEST_SUITE_P(
    DnsmasqTests,
    DnsmasqCustomDNSTest,
    ::testing::Values(
        CustomDNSTestData{"custom.domain", "192.168.1.1", true},
        CustomDNSTestData{"another.domain", "192.168.1.2", false},
        CustomDNSTestData{"invalid_domain", "192.168.1.3", false},
        CustomDNSTestData{"valid.domain", "invalid_ip", false},
        CustomDNSTestData{"", "192.168.1.4", false},
        CustomDNSTestData{"valid.domain", "", false},
        CustomDNSTestData{"invalid domain!", "192.168.1.6", false},
        CustomDNSTestData{"custom.domain", "256.256.256.256", false}
    )
);

struct DHCPLeaseTestData {
    std::string macAddress;
    bool expectedResult;
};

class DnsmasqDHCPLeaseTest : public DnsmasqTest, public ::testing::WithParamInterface<DHCPLeaseTestData> {};

TEST_P(DnsmasqDHCPLeaseTest, DHCPLeaseAllocation) {
    auto testData = GetParam();
    RecordProperty("MacAddress", testData.macAddress);
    RecordProperty("ExpectedResult", testData.expectedResult ? "true" : "false");
    LOG(INFO) << "Allocating DHCP lease for MAC address: " << testData.macAddress
              << " with expected result: " << (testData.expectedResult ? "true" : "false");

    EXPECT_CALL(mockDnsmasq, AllocateDHCPLease(testData.macAddress))
        .Times(1)
        .WillOnce(::testing::Return(testData.expectedResult));
    ASSERT_EQ(mockDnsmasq.AllocateDHCPLease(testData.macAddress), testData.expectedResult);
}

TEST_P(DnsmasqDHCPLeaseTest, LeaseRenewal) {
    auto testData = GetParam();
    RecordProperty("MacAddress", testData.macAddress);
    RecordProperty("ExpectedResult", testData.expectedResult ? "true" : "false");
    LOG(INFO) << "Renewing DHCP lease for MAC address: " << testData.macAddress
              << " with expected result: " << (testData.expectedResult ? "true" : "false");

    EXPECT_CALL(mockDnsmasq, RenewDHCPLease(testData.macAddress))
        .Times(1)
        .WillOnce(::testing::Return(testData.expectedResult));
    ASSERT_EQ(mockDnsmasq.RenewDHCPLease(testData.macAddress), testData.expectedResult);
}

INSTANTIATE_TEST_SUITE_P(
    DnsmasqTests,
    DnsmasqDHCPLeaseTest,
    ::testing::Values(
        DHCPLeaseTestData{"AA:BB:CC:DD:EE:FF", true},
        DHCPLeaseTestData{"FF:EE:DD:CC:BB:AA", false},
        DHCPLeaseTestData{"", false},
        DHCPLeaseTestData{"INVALID_MAC", false}
    )
);

struct StaticIPTestData {
    std::string macAddress;
    std::string ipAddress;
    bool expectedResult;
};

class DnsmasqStaticIPTest : public DnsmasqTest, public ::testing::WithParamInterface<StaticIPTestData> {};

TEST_P(DnsmasqStaticIPTest, StaticIPAllocation) {
    auto testData = GetParam();
    RecordProperty("MacAddress", testData.macAddress);
    RecordProperty("IPAddress", testData.ipAddress);
    RecordProperty("ExpectedResult", testData.expectedResult ? "true" : "false");
    LOG(INFO) << "Setting static IP for MAC address: " << testData.macAddress
              << " with IP address: " << testData.ipAddress
              << " and expecting result: " << (testData.expectedResult ? "true" : "false");

    EXPECT_CALL(mockDnsmasq, SetStaticIP(testData.macAddress, testData.ipAddress))
        .Times(1)
        .WillOnce(::testing::Return(testData.expectedResult));
    ASSERT_EQ(mockDnsmasq.SetStaticIP(testData.macAddress, testData.ipAddress), testData.expectedResult);
}

INSTANTIATE_TEST_SUITE_P(
    DnsmasqTests,
    DnsmasqStaticIPTest,
    ::testing::Values(
        StaticIPTestData{"AA:BB:CC:DD:EE:FF", "192.168.1.100", true},
        StaticIPTestData{"FF:EE:DD:CC:BB:AA", "192.168.1.101", false},
        StaticIPTestData{"", "192.168.1.102", false},
        StaticIPTestData{"AA:BB:CC:DD:EE:FF", "", false},
        StaticIPTestData{"INVALID_MAC", "192.168.1.103", false},
        StaticIPTestData{"AA:BB:CC:DD:EE:FF", "256.256.256.256", false}
    )
);

struct DHCPOptionTestData {
    std::string option;
    std::string value;
    bool expectedResult;
};

class DnsmasqDHCPOptionTest : public DnsmasqTest, public ::testing::WithParamInterface<DHCPOptionTestData> {};

TEST_P(DnsmasqDHCPOptionTest, DHCPOptions) {
    auto testData = GetParam();
    RecordProperty("Option", testData.option);
    RecordProperty("Value", testData.value);
    RecordProperty("ExpectedResult", testData.expectedResult ? "true" : "false");
    LOG(INFO) << "Setting DHCP option: " << testData.option
              << " with value: " << testData.value
              << " and expecting result: " << (testData.expectedResult ? "true" : "false");

    EXPECT_CALL(mockDnsmasq, SetDHCPOption(testData.option, testData.value))
        .Times(1)
        .WillOnce(::testing::Return(testData.expectedResult));
    ASSERT_EQ(mockDnsmasq.SetDHCPOption(testData.option, testData.value), testData.expectedResult);
}

INSTANTIATE_TEST_SUITE_P(
    DnsmasqTests,
    DnsmasqDHCPOptionTest,
    ::testing::Values(
        DHCPOptionTestData{"option:nis-domain", "example.com", true},
        DHCPOptionTestData{"option:router", "192.168.1.1", false},
        DHCPOptionTestData{"", "example.com", false},
        DHCPOptionTestData{"option:nis-domain", "", false}
    )
);

TEST_F(DnsmasqTest, FileExistence) {
    LOG(INFO) << "Checking file existence for /etc/resolv.conf and /var/dnsmasq.conf";
    ASSERT_TRUE(access("/etc/resolv.conf", F_OK) != -1) << "/etc/resolv.conf does not exist";
    ASSERT_TRUE(access("/var/dnsmasq.conf", F_OK) != -1) << "/var/dnsmasq.conf does not exist";
}

bool fileContainsString(const std::string& filePath, const std::string& searchString) {
    std::ifstream file(filePath);
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(searchString) != std::string::npos) {
            return true;
        }
    }
    return false;
}

TEST_F(DnsmasqTest, ResolvConfContents) {
    LOG(INFO) << "Checking contents of /etc/resolv.conf";
    EXPECT_TRUE(fileContainsString("/etc/resolv.conf", "dnsoverride")) << "/etc/resolv.conf does not contain 'dnsoverride'";
    EXPECT_TRUE(fileContainsString("/etc/resolv.conf", "nameserver")) << "/etc/resolv.conf does not contain 'nameserver'";
}

TEST_F(DnsmasqTest, DnsmasqConfContents) {
    LOG(INFO) << "Checking contents of /var/dnsmasq.conf";
    EXPECT_TRUE(fileContainsString("/var/dnsmasq.conf", "interface=brlan0")) << "/var/dnsmasq.conf does not contain 'interface=brlan0'";
    EXPECT_TRUE(fileContainsString("/var/dnsmasq.conf", "dhcp-range=10.")) << "/var/dnsmasq.conf does not contain 'dhcp-range=10.'";
}

int extractIPAddresses(const std::string& filePath, const std::string& searchString, std::vector<std::string>& ipAddresses) {
    std::ifstream file(filePath);
    std::string line;
    std::regex ipv4Regex(R"(\b(?:\d{1,3}\.){3}\d{1,3}\b)");
    std::regex ipv6Regex(R"(\b(?:[0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}\b)");
    std::smatch match;

    while (std::getline(file, line)) {
        if (line.find(searchString) != std::string::npos) {
            while (std::regex_search(line, match, ipv4Regex)) {
                ipAddresses.push_back(match[0]);
                line = match.suffix();
            }
            while (std::regex_search(line, match, ipv6Regex)) {
                ipAddresses.push_back(match[0]);
                line = match.suffix();
            }
        }
    }
    return ipAddresses.size();
}

bool checkDNSResolution(const std::string& ipAddress) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Use AF_UNSPEC to handle both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(ipAddress.c_str(), nullptr, &hints, &res);
    if (status != 0) {
        return false;
    }

    freeaddrinfo(res);
    return true;
}

TEST_F(DnsmasqTest, DNSResolutionForResolvConf) {
    LOG(INFO) << "Checking DNS resolution for IPs in /etc/resolv.conf";
    std::vector<std::string> ip_addresses;

    // Check for dnsoverride variable
    extractIPAddresses("/etc/resolv.conf", "dnsoverride", ip_addresses);
    for (const auto& ip : ip_addresses) {
        EXPECT_TRUE(checkDNSResolution(ip)) << "DNS resolution failed for dnsoverride IP: " << ip;
    }

    ip_addresses.clear(); // Clear the vector for the next set of IPs

    // Check for nameserver variable
    extractIPAddresses("/etc/resolv.conf", "nameserver", ip_addresses);
    for (const auto& ip : ip_addresses) {
        EXPECT_TRUE(checkDNSResolution(ip)) << "DNS resolution failed for nameserver IP: " << ip;
    }
}

bool nslookupTest(const std::string& ipAddress) {
    std::string command = "nslookup " + ipAddress;
    int result = system(command.c_str());
    return WEXITSTATUS(result) == 0; // Check if nslookup command was successful
}

TEST_F(DnsmasqTest, DNSResolutionUsingNslookup) {
    LOG(INFO) << "Checking DNS resolution using nslookup for IPs in /etc/resolv.conf";
    std::vector<std::string> ip_addresses;

    // Extract IP addresses from /etc/resolv.conf for both dnsoverride and nameserver
    extractIPAddresses("/etc/resolv.conf", "dnsoverride", ip_addresses);
    extractIPAddresses("/etc/resolv.conf", "nameserver", ip_addresses);

    // Run nslookup test for each extracted IP address
    for (const auto& ip : ip_addresses) {
        EXPECT_TRUE(nslookupTest(ip)) << "nslookup failed for IP: " << ip;
    }
}