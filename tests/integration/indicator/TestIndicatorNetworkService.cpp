/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <libqtdbustest/DBusTestRunner.h>
#include <libqtdbustest/QProcessDBusService.h>
#include <libqtdbusmock/DBusMock.h>

#include <menuharness/MenuMatcher.h>

#include <NetworkManager.h>

#include <QDebug>
#include <QTestEventLoop>
#include <QSignalSpy>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace QtDBusTest;
using namespace QtDBusMock;

namespace mh = menuharness;

namespace
{
enum class Secure
{
    secure,
    insecure
};

enum class ApMode
{
    infra,
    adhoc
};

enum class ConnectionStatus
{
    connected,
    disconnected
};

class TestIndicatorNetworkService : public Test
{
protected:
    TestIndicatorNetworkService() :
            dbusMock(dbusTestRunner)
    {
    }

    void SetUp() override
    {
        dbusMock.registerNetworkManager();
        // By default the ofono mock starts with one modem
        dbusMock.registerOfono();
        dbusMock.registerURfkill();

        dbusMock.networkManagerInterface();

        dbusTestRunner.startServices();
    }

    void TearDown() override
    {
        QTestEventLoop::instance().enterLoopMSecs(200);
    }

    static mh::MenuMatcher::Parameters phoneParameters()
    {
        return mh::MenuMatcher::Parameters(
                "com.canonical.indicator.network",
                { { "indicator", "/com/canonical/indicator/network" } },
                "/com/canonical/indicator/network/phone");
    }

    void startIndicator()
    {
        indicator.reset(
                new QProcessDBusService("com.canonical.indicator.network",
                                        QDBusConnection::SessionBus,
                                        NETWORK_SERVICE_BIN,
                                        QStringList()));
        indicator->start(dbusTestRunner.sessionConnection());
    }

    static mh::MenuItemMatcher flightModeSwitch(bool toggled = false)
    {
        return mh::MenuItemMatcher::checkbox()
            .label("Flight Mode")
            .action("indicator.airplane.enabled")
            .toggled(toggled);
    }

    static mh::MenuItemMatcher accessPoint(const string& ssid, unsigned int id, Secure secure,
                ApMode apMode, ConnectionStatus connectionStatus)
    {
        return mh::MenuItemMatcher::checkbox()
            .label(ssid)
            .widget("unity.widgets.systemsettings.tablet.accesspoint")
            .action("indicator.accesspoint." + to_string(id))
            .toggled(connectionStatus == ConnectionStatus::connected)
            .string_attribute("x-canonical-wifi-ap-strength-action", "indicator.accesspoint." + to_string(id) + "::strength")
            .boolean_attribute("x-canonical-wifi-ap-is-secure", secure == Secure::secure)
            .boolean_attribute("x-canonical-wifi-ap-is-adhoc", apMode == ApMode::adhoc);
    }

    static mh::MenuItemMatcher wifiEnableSwitch(bool toggled = true)
    {
        return mh::MenuItemMatcher::checkbox()
            .label("Wi-Fi")
            .action("indicator.wifi.enable") // This action is accessed by system-settings-ui, do not change it
            .toggled(toggled);
    }

    static mh::MenuItemMatcher wifiSettings()
    {
        return mh::MenuItemMatcher()
            .label("Wi-Fi settings…")
            .action("indicator.wifi.settings");
    }

    static mh::MenuItemMatcher modemInfo(unsigned int id)
    {
        return mh::MenuItemMatcher()
            .widget("com.canonical.indicator.network.modeminfoitem")
            .string_attribute("x-canonical-modem-roaming-action", "indicator.modem." + to_string(id) + "::roaming")
            .string_attribute("x-canonical-modem-sim-identifier-label-action", "indicator.modem." + to_string(id) + "::sim-identifier-label")
            .string_attribute("x-canonical-modem-connectivity-icon-action", "indicator.modem." + to_string(id) + "::connectivity-icon")
            .string_attribute("x-canonical-modem-status-label-action", "indicator.modem." + to_string(id) + "::status-label")
            .string_attribute("x-canonical-modem-status-icon-action", "indicator.modem." + to_string(id) + "::status-icon")
            .string_attribute("x-canonical-modem-locked-action", "indicator.modem." + to_string(id) + "::locked");
    }

    static mh::MenuItemMatcher modemInfo()
    {
        return mh::MenuItemMatcher()
            .widget("com.canonical.indicator.network.modeminfoitem")
            .has_string_attribute("x-canonical-modem-roaming-action")
            .has_string_attribute("x-canonical-modem-sim-identifier-label-action")
            .has_string_attribute("x-canonical-modem-connectivity-icon-action")
            .has_string_attribute("x-canonical-modem-status-label-action")
            .has_string_attribute("x-canonical-modem-status-icon-action")
            .has_string_attribute("x-canonical-modem-locked-action");
    }

    static mh::MenuItemMatcher cellularSettings()
    {
        return mh::MenuItemMatcher()
            .label("Cellular settings…")
            .action("indicator.cellular.settings");
    }

    DBusTestRunner dbusTestRunner;

    DBusMock dbusMock;

    DBusServicePtr indicator;
};

TEST_F(TestIndicatorNetworkService, BasicMenuContents)
{
    startIndicator();

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .action("indicator.phone.network-status")
            .mode(mh::MenuItemMatcher::Mode::all)
            .submenu()
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(modemInfo(1))
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch())
            .item(wifiSettings())
        ).match());
}

TEST_F(TestIndicatorNetworkService, OneAccessPoint)
{
    auto& networkManager(dbusMock.networkManagerInterface());
    networkManager.AddWiFiDevice("device", "eth1", NM_DEVICE_STATE_DISCONNECTED).waitForFinished();
    networkManager.AddAccessPoint(
            "/org/freedesktop/NetworkManager/Devices/device", "ap", "the ssid",
            "11:22:33:44:55:66", NM_802_11_MODE_INFRA, 0, 0, 's',
            NM_802_11_AP_SEC_KEY_MGMT_PSK).waitForFinished();
    networkManager.AddWiFiConnection(
            "/org/freedesktop/NetworkManager/Devices/device", "connection",
            "the ssid", "wpa-psk").waitForFinished();

    startIndicator();

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .label("")
            .action("indicator.phone.network-status")
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()) // <-- modems are under here
            .item(wifiEnableSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(accessPoint("the ssid", 1,
                      Secure::secure,
                      ApMode::infra,
                      ConnectionStatus::disconnected)
                )
            )
            .item(wifiSettings())
        ).match());
}

TEST_F(TestIndicatorNetworkService, SecondModem)
{
    auto& ofono(dbusMock.ofonoInterface());
    {
        QVariantMap modemProperties {{ "Powered", false } };
        ofono.AddModem("ril_1", modemProperties).waitForFinished();
    }

    startIndicator();

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .label("")
            .action("indicator.phone.network-status")
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(flightModeSwitch())
            .item(mh::MenuItemMatcher()
                .section()
                .item(modemInfo())
                .item(modemInfo())
                .item(cellularSettings())
            )
            .item(wifiEnableSwitch())
            .item(wifiSettings())
        ).match());
}

TEST_F(TestIndicatorNetworkService, FlightModeTalksToURfkill)
{
    startIndicator();

    auto& urfkillInterface = dbusMock.urfkillInterface();
    QSignalSpy urfkillSpy(&urfkillInterface, SIGNAL(FlightModeChanged(bool)));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(flightModeSwitch(false)
                .activate() // <--- Activate the action now
            )
        ).match());

    // Wait to be notified that flight mode was enabled
    ASSERT_TRUE(urfkillSpy.wait());
    EXPECT_EQ(urfkillSpy.first(), QVariantList() << QVariant(true));
}

TEST_F(TestIndicatorNetworkService, IndicatorListensToURfkill)
{
    startIndicator();

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .label("")
            .action("indicator.phone.network-status")
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .submenu()
            .item(flightModeSwitch(false))
        ).match());

    ASSERT_TRUE(dbusMock.urfkillInterface().FlightMode(true));

    EXPECT_MATCHRESULT(mh::MenuMatcher(phoneParameters())
        .item(mh::MenuItemMatcher()
            .label("")
            .action("indicator.phone.network-status")
            .mode(mh::MenuItemMatcher::Mode::starts_with)
            .item(flightModeSwitch(true))
        ).match());
}

} // namespace
