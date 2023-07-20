/*
    small Pinephone QML interface for Out-Of-Band communication.

    Copyright (C) 2023 Resilience Theatre

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; If not, see <http://www.gnu.org/licenses/>.

    TODO:
    ---------------------------------------------------------------------
    [ ]     Wifi SSID's with spaces do not work

    NOTE:   If you turn LTE modem off with DIP switches, adjust /root/utils/cell.sh
            because it might block if modem is non reachable.

            QT_ASSUME_STDERR_HAS_CONSOLE=1

            Random pick from Internet:

            "Light source of wavelength between 620nm and 700nm
            will be visible to people with normal red vision,
            but will have no effect on scotopic retina cell,
            thereby preserving night vision."

            https://academo.org/demos/wavelength-to-colour-relationship/

            This code is highly experimental.

    [1] https://www.cablefree.net/wirelesstechnology/4glte/rsrp-rsrq-measurement-lte/
    [2] http://manuel.reithuber.net/2013/01/interface-for-default-route-in-qt-on-linux/
*/

#include "engineclass.h"
#include <QDebug>
#include <QFile>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QCoreApplication>
#include <QTimer>
#include <QSettings>
#include <QQmlEngine>
#include <QQmlComponent>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#define PRE_VAULT_INI_FILE      "/opt/prevault.ini"
#define USER_PREF_INI_FILE      "/opt/tunnel/userpreferences.ini"
#define TELEMETRY_FIFO_IN       "/tmp/telemetry_fifo_in"
#define TELEMETRY_FIFO_OUT      "/tmp/telemetry_fifo_out"
#define MESSAGE_RECEIVE_FIFO    "/tmp/message_fifo_out"
#define VAULT_SELECTOR_INI_FILE "/mnt/vaults.ini"

#define NODECOUNT               10
#define CONNPOINTCOUNT          3
#define INDICATE_ONLY           0
#define LOG_ONLY                1
#define LOG_AND_INDICATE        2
#define SETTINGS_INI_FILE       "/opt/tunnel/sinm.ini"
#define SUBSTITUTE_CHAR_CODE    24
#define PWR_GPIO_INPUT_PATH     "/dev/input/by-path/platform-1f03400.rsb-platform-axp221-pek-event"
#define VOL_GPIO_INPUT_PATH     "/dev/input/by-path/platform-1c21800.lradc-event"
#define HF_PLUG_GPIO_INPUT_PATH "/dev/input/by-path/platform-sound-event"
#define BATTERY_CAPACITY_PATH   "/sys/class/power_supply/axp20x-battery/capacity"
#define BATTERY_CURRENT_PATH    "/sys/class/power_supply/axp20x-battery/current_now"
#define BATTERY_STATUS_PATH     "/sys/class/power_supply/axp20x-battery/status"
#define PROXIMITY_SENSOR_PATH   "/sys/devices/platform/soc/1c2b000.i2c/i2c-1/1-0048/iio:device1/in_proximity_raw"
#define PROXIMITY_SENSOR_PATH_2 "/sys/devices/platform/soc/1c2b000.i2c/i2c-1/1-0048/iio:device2/in_proximity_raw"
#define LOCK_DEVICE             true
#define UNLOCK_DEVICE           false
#define DEVICE_LOCK_TIME        120
#define FIFO_TIMEOUT            1
#define FIFO_REPLY_RECEIVED     0
#define SIDEBUTTON_KEY_UP       0
#define SIDEBUTTON_KEY_DOWN     1
#define IWD_MAIN_CONFIG_FILE    "/etc/iwd/main.conf"

#define AUTOMATIC_SHUTDOWNTIME   600000 // 10 min
#define AUTOMATIC_SHUTDOWNTIME_IN_VAULT_MODE   120000 // 2 min

engineClass::engineClass(QObject *parent)
    : QObject{parent}
{
    /* Initial UI colors */
    mMainColor = "#00FF00";
    emit mainColorChanged();
    mHighColor = "lightgreen";
    emit highColorChanged();
    mDimColor = "#21cc00";
    emit dimColorChanged();
    mMessageColorLocal = "#FFFFFF";
    mMessageColorRemote = "#00FF00";

    QTimer::singleShot(2 * 1000, this, SLOT(loadSettings()));
    QTimer::singleShot(4 * 1000, this, SLOT(initEngine()));

    /* environment and proximity evaluation timers */
    envTimer = new QTimer();
    connect(envTimer, &QTimer::timeout, this, QOverload<>::of(&engineClass::envTimerTick));
    envTimer->start(5000);
    proximityTimer = new QTimer();
    connect(proximityTimer, &QTimer::timeout, this, QOverload<>::of(&engineClass::proximityTimerTick));
    proximityTimer->start(2000);
    automaticShutdownTimer = new QTimer();
    connect(automaticShutdownTimer, &QTimer::timeout, this, QOverload<>::of(&engineClass::automaticShutdownTimeout));

    /* Power button, TODO: m_pwrButtonFileHandle close */
    QByteArray pwrButtonDevice = QByteArrayLiteral(PWR_GPIO_INPUT_PATH);
    m_pwrButtonFileHandle = open(pwrButtonDevice.constData(), O_RDONLY);
    if (m_pwrButtonFileHandle >= 0) {
        m_pwrButtonNotify = new QSocketNotifier(m_pwrButtonFileHandle, QSocketNotifier::Read, this);
        connect(m_pwrButtonNotify, SIGNAL(activated(int)), this, SLOT(readPwrGpioButton()));
    } else {
        qErrnoWarning(errno, "Cannot open input device %s", pwrButtonDevice.constData());
    }
    /* Volume buttons */
    QByteArray volButtonDevice = QByteArrayLiteral(VOL_GPIO_INPUT_PATH);
    m_volButtonFileHandle = open(volButtonDevice.constData(), O_RDONLY);
    if (m_volButtonFileHandle >= 0) {
        m_volButtonNotify = new QSocketNotifier(m_volButtonFileHandle, QSocketNotifier::Read, this);
        connect(m_volButtonNotify, SIGNAL(activated(int)), this, SLOT(readVolGpioButton()));
    } else {
        qErrnoWarning(errno, "Cannot open input device %s", volButtonDevice.constData());
    }
    /* Headset insert  */
    QByteArray hfPlugDevice = QByteArrayLiteral(HF_PLUG_GPIO_INPUT_PATH);
    m_hfPlugFileHandle = open(hfPlugDevice.constData(), O_RDONLY);
    if (m_hfPlugFileHandle >= 0) {
        m_hfPlugNotify = new QSocketNotifier(m_hfPlugFileHandle, QSocketNotifier::Read, this);
        connect(m_hfPlugNotify, SIGNAL(activated(int)), this, SLOT(readHfPlugEvents()));
    } else {
        qErrnoWarning(errno, "Cannot open input device %s", hfPlugDevice.constData());
    }
    /* Enable backlight */
    runExternalCmd("/bin/pptk-backlight", {"set_percent", "50"});
    m_screenTimeoutCounter=DEVICE_LOCK_TIME;
    /* Set default wifi status on top bar*/
    m_wifiNotifyText = "WIFI";
    m_wifiNotifyColor = mMainColor;
    emit wifiNotifyTextChanged();
    emit wifiNotifyColorChanged();
    /* Load about text content */
    loadAboutText();
    mAudioDeviceBusy = false;
    /* No nuke */
    mNukeCounterVisible = false;
    emit nukeCounterVisibleChanged();
    /* Internal speaker by default */
    mMixerNameSpeaker = "Earpiece";
    runExternalCmd("/root/utils/internal-speaker.sh", {});
    mHfIndicatorVisible = false;
    emit hfIndicatorVisibleChanged();
    mMacsecKeyed = "NO KEY";
    emit macsecKeyedChanged();
    mMacsecKeyValid = false;
    emit macsecValidChanged();
    m_busyIndicatorActive = false;
    emit busyIndicatorChanged();

    loadVaultPreferences();
}

void engineClass::automaticShutdownTimeout()
{
    powerOff();
}


QString engineClass::appVersion()
{
    return APP_VERSION;
}

QString engineClass::getMainColor()
{
    return mMainColor;
}

QString engineClass::getHighColor()
{
    return mHighColor;
}

QString engineClass::getDimColor()
{
    return mDimColor;
}

bool engineClass::getNukeCounterVisible()
{
    return mNukeCounterVisible;
}

QString engineClass::getNukeCounterText()
{
    return mNukeCounterText;
}

bool engineClass::getHfIndicatorVisible()
{
    return mHfIndicatorVisible;
}

void engineClass::runExternalCmd(QString command, QStringList parameters){
    qint64 pid;
    QProcess process;
    process.setProgram(command);
    process.setArguments(parameters);
    process.startDetached(&pid);
}

void engineClass::runExternalCmdCaptureOutput(QString command, QStringList parameters){
    QProcess process;
    process.setProgram(command);
    process.setArguments(parameters);
    process.start();
    process.waitForFinished();
    QString result=process.readAllStandardOutput();
    qDebug() << "runExternalCmdCaptureOutput() " << result;
}

void engineClass::lockDevice(bool state)
{
    if ( state == LOCK_DEVICE && g_connectState == false ) {
        m_deviceLocked = true;
        runExternalCmd("/bin/pptk-vibrate", {"200","200","1"});
        runExternalCmd("/bin/pptk-backlight", {"set_percent", "0"});
        runExternalCmd("/bin/pptk-cpu-sleep", {"enable"});
        m_touchBlock_active=true;
        emit touchBlock_activeChanged();
        m_SwipeViewIndex = 0;
        emit swipeViewIndexChanged();
        m_lockScreen_active=true;
        emit lockScreen_activeChanged();
        m_lockScreenPinCode = "";
        emit lockScreenPinCodeChanged();
        m_camoScreen_active = false;
        emit camoScreen_activeChanged();
        if ( m_deepSleepEnabled )
            runExternalCmd("/bin/deepsleep.sh", {});
    }
    if ( state == UNLOCK_DEVICE ) {
        m_deviceLocked = false;
        m_screenTimeoutCounter=DEVICE_LOCK_TIME;
        runExternalCmd("/bin/pptk-vibrate", {"200","200","2"});
        runExternalCmd("/bin/pptk-backlight", {"set_percent", "50"});
        runExternalCmd("/bin/pptk-cpu-sleep", {"disable"});
        m_touchBlock_active=false;
        emit touchBlock_activeChanged();
    }
}

bool engineClass::getPowerOffVisible()
{
    return mPowerOffDialog;
}

void engineClass::readPwrGpioButtonTimer()
{
    if ( m_deviceLocked == false ) {
        if ( mPwrButtonCycle && mPwrButtonReleased == false ) {
            qDebug() << "Power button held 2 s detected";
            m_SwipeViewIndex = 0;
            emit swipeViewIndexChanged();
            mPowerOffDialog=true;
            emit powerOffVisibleChanged();
        }
    }
}

void engineClass::closePowerOffDialog()
{
    mPowerOffDialog=false;
    emit powerOffVisibleChanged();
}

void engineClass::setSwipeIndex(int index)
{
    m_SwipeViewIndex = index;
}

void engineClass::readPwrGpioButton()
{
    struct input_event in_ev = { 0 };
    /*  Loop read data  */
    if (sizeof(struct input_event) != read(m_pwrButtonFileHandle, &in_ev, sizeof(struct input_event))) {
        perror("read error");
        exit(-1);
    }
    switch (in_ev.type) {
    case EV_KEY:
    {
        if (KEY_POWER == in_ev.code && in_ev.value == 1 ) {
            mPwrButtonCycle = true;
            mPwrButtonReleased = false;
            QTimer::singleShot(4 * 1000, this, SLOT(readPwrGpioButtonTimer()));
            runExternalCmd("/bin/pptk-led", {"set", "red", "1"});
            break;
        }
        if (KEY_POWER == in_ev.code && in_ev.value == 0 ) {
            mPwrButtonReleased = true;
            runExternalCmd("/bin/pptk-led", {"set", "red", "0"});
            if ( m_deviceLocked == false && !mPowerOffDialog ) {
                lockDevice(LOCK_DEVICE);
            } else
            {
                lockDevice(UNLOCK_DEVICE);
            }
            break;
        }
    }
    }
}

/* Nuke down counter as long as Volume UP button is pressed down */
void engineClass::countNukeTimer()
{
    mNukeCounterText = QString::number(nukeCountDownValue);
    mNukeCounterVisible = true;
    emit nukeCounterTextChanged();
    emit nukeCounterVisibleChanged();
    runExternalCmd("/bin/pptk-vibrate", {"400","10","1"});
    if ( nukeCountDownValue > 0 ) {
        nukeCountDownValue--;
    }
    /* TODO: When nukeCountDownValue == 0 do actual nuke */
    /* Abort nuke counter if volume up is released */
    if ( mVolUpKeyReleased == true ) {
        nukeCountDownTimer->stop();
        mNukeCounterVisible = false;
        emit nukeCounterVisibleChanged();
    }
}

/* Get default route interface [2] */
QString engineClass::getDefaultRoute()
{
    QFile routeFile("/proc/net/route");
    QString rc;

    if (!routeFile.open(QFile::ReadOnly))
        qDebug() << "Error getting route information " << routeFile.errorString();

    QByteArray line;
    while (!(line = routeFile.readLine()).isNull()) {
      QList<QByteArray> parts = line.split('\t');
      QByteArray intf = parts[0];
      QByteArray route = parts[1];
      QByteArray mask = parts[7];
      // Find make sure the destination address is 0.0.0.0 and the netmask empty
      if (route == "00000000" && mask == "00000000") {
        rc = intf;
        break;
      }
    }
    return rc;
}

/* pre-timer for nuke down counting */
void engineClass::readNukeTimer()
{
    if ( mVolUpKeyReleased  == false ) {
        if ( mNukeTimerRunning ) {
            mNukeTimerRunning = false;
            runExternalCmd("/bin/pptk-backlight", {"set_percent", "50"});
            nukeCountDownValue = 10;
            nukeCountDownTimer = new QTimer();
            connect(nukeCountDownTimer, &QTimer::timeout, this, QOverload<>::of(&engineClass::countNukeTimer));
            nukeCountDownTimer->start(1000);
        }
    }
    mNukeTimerRunning=false;
}

/* Volume up/down:
    * On settings page adjusts backlight
    * Other pages, adjusts volume
    * 5 s press on volume up activates nuke timer
    * WiP: PTT when mMacsecPttEnabled
*/
void engineClass::readVolGpioButton()
{
    struct input_event in_ev = { 0 };

    /*  Loop read data  */
    if (sizeof(struct input_event) != read(m_volButtonFileHandle, &in_ev, sizeof(struct input_event))) {
        perror("read error");
        exit(-1);
    }
    switch (in_ev.type) {
    case EV_KEY:
    {
        if ( in_ev.code == KEY_VOLUMEDOWN && in_ev.value == SIDEBUTTON_KEY_DOWN ) {
            if ( mMacsecPttEnabled ) {
                /* TODO: activate PTT */
                qDebug() << "PTT down!";
                break;
            } else {
                /* Brightness */
                if ( m_SwipeViewIndex == 2 ) {
                    if ( mBacklightLevel >= 20 && mBacklightLevel <= 90 ) {
                        registerTouch();
                        runExternalCmd("/bin/pptk-backlight", {"set_percent", QString::number(mBacklightLevel)});
                        mBacklightLevel = mBacklightLevel - 10;
                    }
                } else {
                    /* Volume */
                    if ( m_SpeakerVolumeRuntimeValue > 0 && m_SpeakerVolumeRuntimeValue <= 100 )
                    {
                        m_SpeakerVolumeRuntimeValue = m_SpeakerVolumeRuntimeValue - 5;
                    }
                    m_statusMessage = "Volume: " + QString::number(m_SpeakerVolumeRuntimeValue) + " %" ;
                    emit statusMessageChanged();
                    uPref.volumeValue = QString::number(m_SpeakerVolumeRuntimeValue);
                    saveUserPreferences();
                    break;
                }
            }
        }
        if ( in_ev.code == KEY_VOLUMEDOWN && in_ev.value == SIDEBUTTON_KEY_UP ) {
            if ( mMacsecPttEnabled ) {
                /* TODO: de-activate PTT */
                qDebug() << "PTT released";
                break;
            }
        }

        if (in_ev.code == KEY_VOLUMEUP && in_ev.value == SIDEBUTTON_KEY_DOWN ) {
            /* Start pre-timer  */
            mVolUpKeyReleased = false;
            if ( !mNukeTimerRunning) {
                QTimer::singleShot(5 * 1000, this, SLOT(readNukeTimer()));
                mNukeTimerRunning = true;
            }
            /* Brightness */
            if ( m_SwipeViewIndex == 2 ) {
                if ( mBacklightLevel >= 10 && mBacklightLevel <= 80 ) {
                    registerTouch();
                    mBacklightLevel = mBacklightLevel + 10;
                    runExternalCmd("/bin/pptk-backlight", {"set_percent", QString::number(mBacklightLevel)});
                }
            } else {
                /* Volume */
                if ( m_SpeakerVolumeRuntimeValue >= 0 && m_SpeakerVolumeRuntimeValue < 100 )
                {
                    m_SpeakerVolumeRuntimeValue = m_SpeakerVolumeRuntimeValue + 5;
                }
                m_statusMessage = "Volume: " + QString::number(m_SpeakerVolumeRuntimeValue) + " %" ;
                emit statusMessageChanged();
                uPref.volumeValue = QString::number(m_SpeakerVolumeRuntimeValue);
                saveUserPreferences();
                break;
            }
        }
        if ( in_ev.code == KEY_VOLUMEUP  && in_ev.value == SIDEBUTTON_KEY_UP  ) {
            mVolUpKeyReleased = true;
        }
    }
    }
}

/* 3.5 mm headphone detection */
void engineClass::readHfPlugEvents()
{
    struct input_event in_ev = { 0 };
    /*  Loop read data  */
    if (sizeof(struct input_event) != read(m_hfPlugFileHandle, &in_ev, sizeof(struct input_event))) {
        perror("read error");
        exit(-1);
    }
    switch (in_ev.type) {
    case EV_SW:
    {
        if (SW_HEADPHONE_INSERT == in_ev.code && in_ev.value == 1 ) {
            mMixerNameSpeaker = "Headphone";
            runExternalCmd("/root/utils/headset.sh", {});
            mHfIndicatorVisible = true;
            emit hfIndicatorVisibleChanged();
            break;
        }
        if (SW_HEADPHONE_INSERT == in_ev.code && in_ev.value == 0 ) {
            mMixerNameSpeaker = "Earpiece";
            runExternalCmd("/root/utils/internal-speaker.sh", {});
            mHfIndicatorVisible = false;
            emit hfIndicatorVisibleChanged();
            break;
        }
    }
    }
}

/*
 * Set system volume with external process.
 *
 * * Mixer is now named based on HF plug status.
 *
 * Playback volume:     amixer sset [DEVICENAME] Playback 100%
 * Microphone volume:   amixer sset [DEVICENAME] Capture 5%+
 *
 * Internal earpiece: "'Earpiece'"
 * Internal mic: "'Mic1'" or "'Mic2'"
 *
 */
void engineClass::setSystemVolume(int volume)
{
    QString volumePercentString = QString::number(volume) + "%";
    qint64 pid;
    QProcess process;
    process.setProgram("/usr/bin/amixer");
    process.setArguments({"sset",mMixerNameSpeaker, "Playback",volumePercentString});
    process.setStandardOutputFile(QProcess::nullDevice());
    process.setStandardErrorFile(QProcess::nullDevice());
    process.startDetached(&pid);
}
void engineClass::setMicrophoneVolume(int volume)
{
    QString volumePercentString = QString::number(volume) + "%";
    qint64 pid;
    QProcess process;
    process.setProgram("/usr/bin/amixer");
    process.setArguments({"sset","'Mic'", "Capture", volumePercentString });
    process.setStandardOutputFile(QProcess::nullDevice());
    process.setStandardErrorFile(QProcess::nullDevice());
    process.startDetached(&pid);
}
void engineClass::loadUserPreferences()
{
    QSettings settings(USER_PREF_INI_FILE,QSettings::IniFormat);
    uPref.volumeValue = settings.value("volume","70").toString();
    uPref.m_micVolume = settings.value("micvolume","100").toString();
    nodes.beepActive = settings.value("beep").toString();
    m_SpeakerVolumeRuntimeValue = uPref.volumeValue.toInt();
    setSystemVolume(m_SpeakerVolumeRuntimeValue);
    setMicrophoneVolume( uPref.m_micVolume.toInt() );
    uPref.m_pinCode = settings.value("pincode","4321").toString();
    m_deepSleepEnabled = settings.value("deepsleep",false).toBool();
    emit deepSleepEnabledChanged();
    m_lteEnabled = settings.value("lte",false).toBool();
    emit lteEnabledChanged();
    m_lteCellDisplayEnabled=settings.value("celldisplay",false).toBool();
    emit lteCellDisplayEnabledChanged();
    m_nightModeEnabled = settings.value("nightmode",false).toBool();
    emit nightModeEnabledChanged();
    mMacsecPttEnabled = settings.value("ptt",false).toBool();
    emit macsecPttEnabledChanged();
    mLayer2WifiEnabled = settings.value("layer2wifi",false).toBool();
    emit layer2WifiChanged();
    // Some settings are required to be available before vault is open,
    // so we load them from PRE_VAULT_INI_FILE
    QSettings vaultPreferences(PRE_VAULT_INI_FILE,QSettings::IniFormat);
    m_callSignVisibleOnVaultPage = vaultPreferences.value("vaultpagecallsign",true).toBool();
    emit callSignOnVaultEnabledChanged();
    m_messageEraseEnabled = vaultPreferences.value("msg_erase",true).toBool();
    emit messageEraseEnabledChanged();
    m_automaticShutdownEnabled = vaultPreferences.value("automaticshutdown",true).toBool();
    emit automaticShutdownEnabledChanged();
    if (m_automaticShutdownEnabled) {
        automaticShutdownTimer->start( AUTOMATIC_SHUTDOWNTIME );
    }


    if ( m_nightModeEnabled ) {
                                    //  625 nm      Green           640 nm
        mMainColor = "#ff2100";     //  #ff6300     #00FF00         ff2100
        emit mainColorChanged();
        mHighColor = "#ff8080";     //  #ffa300     lightgreen      ff6100
        emit highColorChanged();
        mDimColor = "#cc2100";      //  #dd5300     21cc00          cc2100
        emit dimColorChanged();
        m_wifiNotifyColor = mMainColor;
        emit wifiNotifyColorChanged();
        mMessageColorLocal = mDimColor;
        mMessageColorRemote = mMainColor;

    }
    else {
                                    //  625 nm      Green           640 nm
        mMainColor = "#00FF00";     //  #ff6300     #00FF00         ff2100
        emit mainColorChanged();
        mHighColor = "lightgreen";  //  #ffa300     lightgreen      ff6100
        emit highColorChanged();
        mDimColor = "#21cc00";      //  #dd5300     21cc00          cc2100
        emit dimColorChanged();
        m_wifiNotifyColor = mMainColor;
        emit wifiNotifyColorChanged();
        mMessageColorLocal = mDimColor;
        mMessageColorRemote = mMainColor;
    }
}

void engineClass::saveUserPreferences()
{
    QSettings settings(USER_PREF_INI_FILE,QSettings::IniFormat);
    settings.setValue("volume", uPref.volumeValue);
    setSystemVolume( uPref.volumeValue.toInt() );
    setMicrophoneVolume(uPref.m_micVolume.toInt());
}

/* Read /mnt/vaults.ini for vault names on selector */
void engineClass::loadVaultPreferences()
{
    int loop=0;
    QSettings settings(VAULT_SELECTOR_INI_FILE,QSettings::IniFormat);
    int vaultCount = settings.value("vault_count","0").toInt();
    m_vaultNames.clear();
    for ( loop=0; loop<vaultCount; loop++)
    {
        m_vaultNames << settings.value("vault_" + QString::number(loop) + "_name","").toString();
    }
    emit vaultNamesChanged();
}


void engineClass::loadSettings()
{
    if ( m_vaultModeActive ) {
        automaticShutdownTimer->start( AUTOMATIC_SHUTDOWNTIME_IN_VAULT_MODE );
        QSettings vaultPreferences(PRE_VAULT_INI_FILE,QSettings::IniFormat);
        bool vaultPinDisplay = vaultPreferences.value("vaultpagecallsign",false).toBool();
        if ( vaultPinDisplay ) {
            QString vaultMyCallSign = vaultPreferences.value("my_name","").toString();
            m_vaultNotifyText = "ENTER VAULT PIN [ " + vaultMyCallSign + " ]";
            emit vaultScreenNotifyTextChanged();
        } else {
            m_vaultNotifyText = "SELECT VAULT & INPUT PIN";
            emit vaultScreenNotifyTextChanged();
        }
        m_vaultNotifyColor = "red";
        m_vaultNotifyTextColor = "white";
        emit vaultScreenNotifyColorChanged();
        emit vaultScreenNotifyTextColorChanged();
        return;
    }
    /* Load volumes (for now)*/
    loadUserPreferences();
    QSettings settings(SETTINGS_INI_FILE,QSettings::IniFormat);
    /* Get own node information*/
    nodes.myNodeId = settings.value("my_id").toString();
    nodes.myNodeIp = settings.value("my_ip").toString();
    nodes.myNodeName = settings.value("my_name").toString();
    emit myCallSignChanged();
    /* Get nodes */
    for (int x=0; x < NODECOUNT; x++ ) {
        nodes.node_name[x] = settings.value("node_name_"+QString::number(x), "").toString();
        nodes.node_ip[x] = settings.value("node_ip_"+QString::number(x), "").toString();
        nodes.node_id[x] = settings.value("node_id_"+QString::number(x), "").toString();
    }
    /* Change button titles */
    m_peer_0_CallSign=nodes.node_name[0];
    emit peer_0_NameChanged();
    m_peer_1_CallSign=nodes.node_name[1];
    emit peer_1_NameChanged();
    m_peer_2_CallSign=nodes.node_name[2];
    emit peer_2_NameChanged();
    m_peer_3_CallSign=nodes.node_name[3];
    emit peer_3_NameChanged();
    m_peer_4_CallSign=nodes.node_name[4];
    emit peer_4_NameChanged();
    m_peer_5_CallSign=nodes.node_name[5];
    emit peer_5_NameChanged();
    m_peer_6_CallSign=nodes.node_name[6];
    emit peer_6_NameChanged();
    m_peer_7_CallSign=nodes.node_name[7];
    emit peer_7_NameChanged();
    m_peer_8_CallSign=nodes.node_name[8];
    emit peer_8_NameChanged();
    m_peer_9_CallSign=nodes.node_name[9];
    emit peer_9_NameChanged();

    m_statusMessage = "Settings loaded, please wait.";
    emit statusMessageChanged();

    // Key usage population
    reloadKeyUsage();

    // Set peer contact colors
    m_peer_0_CallSignColor = mMainColor;
    emit peer_0_NameColorChanged();
    m_peer_1_CallSignColor = mMainColor;
    emit peer_1_NameColorChanged();
    m_peer_2_CallSignColor = mMainColor;
    emit peer_2_NameColorChanged();
    m_peer_3_CallSignColor = mMainColor;
    emit peer_3_NameColorChanged();
    m_peer_4_CallSignColor = mMainColor;
    emit peer_4_NameColorChanged();
    m_peer_5_CallSignColor = mMainColor;
    emit peer_5_NameColorChanged();
    m_peer_6_CallSignColor = mMainColor;
    emit peer_6_NameColorChanged();
    m_peer_7_CallSignColor = mMainColor;
    emit peer_7_NameColorChanged();
    m_peer_8_CallSignColor = mMainColor;
    emit peer_8_NameColorChanged();
    m_peer_9_CallSignColor = mMainColor;
    emit peer_9_NameColorChanged();
    // Load APN
    loadApnName();
}

void engineClass::setVaultMode(bool vaultModeActive) {
    m_vaultModeActive=vaultModeActive;
    emit vaultScreen_activeChanged();
}

/* This function will read key file lenghts and counter values of each key.
   Way keys are named as files, depends on index unit has. Therefore we
   need 'tipping point' - which gives index order change location while
   checking files. See key creation code to get better picture of this.
*/
void engineClass::reloadKeyUsage()
{
    int tippingPoint=0;
    QString keyfile;
    QString keyCountfile;
    for (int x=0; x < NODECOUNT; x++ ) {
        m_keyPersentage_incount[x] = "";
        m_keyPersentage_outcount[x] = "";
    }
    /* Find a tipping point in keyfile naming. */
    for (int x=0; x < NODECOUNT; x++ ) {
        if ( nodes.node_id[x].compare( nodes.myNodeId ) == 0 ){
            tippingPoint = x;
        }
    }
    /* Loop for inkey with tipping point evaluation */
    for (int x=0; x < NODECOUNT; x++ ) {
        if ( nodes.node_id[x].compare( nodes.myNodeId ) != 0 )
        {
            if ( x < tippingPoint ) {
                keyfile = "/opt/tunnel/" + nodes.node_id[x] + nodes.myNodeId +".inkey";
                keyCountfile = "/opt/tunnel/" + nodes.node_id[x] + nodes.myNodeId +".incount";
            } else {
                keyfile = "/opt/tunnel/" + nodes.myNodeId + nodes.node_id[x] +".inkey";
                keyCountfile = "/opt/tunnel/" + nodes.myNodeId + nodes.node_id[x] +".incount";
            }
            // Read actual information
            long int key_file_size = get_file_size(keyfile);
            long int rx_key_used = get_key_index(keyCountfile);
            float key_percentage = (100.0*rx_key_used)/key_file_size;
            QString fullPercentage=QString::number(100-key_percentage,'f',0);
            m_keyPersentage_incount[x] = fullPercentage;
        }
    }
    /* Loop for outkey with tipping point evaluation */
    for (int x=0; x < NODECOUNT; x++ ) {
        if ( nodes.node_id[x].compare( nodes.myNodeId ) != 0 )
        {
            if ( x < tippingPoint ) {
                keyfile = "/opt/tunnel/" + nodes.node_id[x] + nodes.myNodeId +".outkey";
                keyCountfile = "/opt/tunnel/" + nodes.node_id[x] + nodes.myNodeId +".outcount";
            } else {
                keyfile = "/opt/tunnel/" + nodes.myNodeId + nodes.node_id[x] +".outkey";
                keyCountfile = "/opt/tunnel/" + nodes.myNodeId + nodes.node_id[x] +".outcount";
            }
            // Read actual information
            long int key_file_size = get_file_size(keyfile);
            long int rx_key_used = get_key_index(keyCountfile);
            float key_percentage = (100.0*rx_key_used)/key_file_size;
            QString fullPercentage=QString::number(100-key_percentage,'f',0);
            m_keyPersentage_outcount[x] = fullPercentage;
        }
    }
    emit peer_0_keyPercentageChanged();
    emit peer_1_keyPercentageChanged();
    emit peer_2_keyPercentageChanged();
    emit peer_3_keyPercentageChanged();
    emit peer_4_keyPercentageChanged();
    emit peer_5_keyPercentageChanged();
    emit peer_6_keyPercentageChanged();
    emit peer_7_keyPercentageChanged();
    emit peer_8_keyPercentageChanged();
    emit peer_9_keyPercentageChanged();
}

// TODO: Check i2c path change and implement better solution
void engineClass::proximityTimerTick()
{
    bool proxValueAvailable = false;
    QString proxLine;
    QString proxFileName=PROXIMITY_SENSOR_PATH;
    QFile proxFile(proxFileName);
    if(!proxFile.exists()){
    } else {
        if (proxFile.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream stream(&proxFile);
            while (!stream.atEnd()){
                proxLine = stream.readLine();
                break;
            }
        }
        proxFile.close();
        proxValueAvailable = true;
    }

    if ( !proxValueAvailable ) {
        proxFileName = PROXIMITY_SENSOR_PATH_2;
        QFile proxFileSensodary(proxFileName);
        if(!proxFileSensodary.exists()){
            return;
        } else {
            if (proxFileSensodary.open(QIODevice::ReadOnly | QIODevice::Text)){
                QTextStream stream(&proxFileSensodary);
                while (!stream.atEnd()){
                    proxLine = stream.readLine();
                    break;
                }
            }
            proxFileSensodary.close();
            proxValueAvailable = true;
        }
    }

    if ( proxValueAvailable ) {
        if ( proxLine.toInt() > 400 ) {
            // Swipe to front page & block touch
            m_SwipeViewIndex = 0;
            emit swipeViewIndexChanged();
            m_touchBlock_active=true;
            emit touchBlock_activeChanged();

        } else {
            // Unblock touch
            m_touchBlock_active=false;
            emit touchBlock_activeChanged();
        }
    }

}

void engineClass::envTimerTick()
{
/*
Cell information

    PLMN: [MCC] + [MNC] (Mobile operator identifier)
    Tracking Area Code: '211'
    Global Cell ID: '2205450'
    Serving Cell ID: '51'
    EUTRA Absolute RF Channel Number

Signal information

    RSSI – received Signal Strength Indicator
    RSRQ – reference Signal Received Quality
    RSRP - reference Signal Received Power
    SNR  - signal-to-noise ratio

*/

    if ( m_vaultModeActive ) {
        return;
    }

    /* Get default route interface */
    if ( 1 ) {
        mDefaultRouteInterface = getDefaultRoute();
        if ( mDefaultRouteInterface.contains("wwan0") ) {
            m_wifiNotifyText = "LTE";
            m_wifiNotifyColor = mMainColor;
            emit wifiNotifyTextChanged();
            emit wifiNotifyColorChanged();
        }
        if ( mDefaultRouteInterface.contains("wlan0") ) {
            m_wifiNotifyText = "WIFI";
            m_wifiNotifyColor = mMainColor;
            emit wifiNotifyTextChanged();
            emit wifiNotifyColorChanged();
        }
    }

    if ( 1 ) {
        /* Read voltage from ENV file as volts */
        QString filename="/tmp/env";
        QFile file(filename);
        if(!file.exists()){
            qDebug() << "Error, no file: "<<filename;
        }
        QString line;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream stream(&file);
            while (!stream.atEnd()){
                line = stream.readLine();
            }
        }
        file.close();

        /* Split string and display voltage type #1 (env file / volts)  */
        QStringList elements = line.split(',');
        mVoltage = elements[0] + " V";
        emit voltageValueChanged();
        float voltageCompareValue = elements[0].toFloat();
        if ( voltageCompareValue < 3.7 && voltageCompareValue > 3.6 ) {
            mvoltageNotifyColor = "#FFFF00";
            emit voltageNotifyColorChanged();
        }
        if (voltageCompareValue > 3.7) {
            mvoltageNotifyColor = mMainColor;
            emit voltageNotifyColorChanged();
        }
        if (voltageCompareValue < 3.6) {
            mvoltageNotifyColor = "#FF5555";
            emit voltageNotifyColorChanged();
        }
        /* Parse cellular environment if we have it */
        mPlmn = elements[1];
        mTa = elements[2];
        mGc = elements[3];
        mSc = elements[4];
        mAc = elements[5];
        mRssi = elements[6];
        mRsrq = elements[7];
        mRsrp = elements[8];
        mSnr = elements[9];
        emit plmnChanged();
        emit taChanged();
        emit gcChanged();
        emit scChanged();
        emit acChanged();
        emit rssiChanged();
        emit rsrqChanged();
        emit rsrpChanged();
        emit snrChanged();
    }
    /* Read capacity as percentage from /sys/class/power_supply/axp20x-battery/capacity */
    if ( 1 ) {
        QString chargeStatusText="";
        QFile batStatusFile(BATTERY_STATUS_PATH);
        if(!batStatusFile.exists()){
        }
        QString batStatus;
        if (batStatusFile.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream stream(&batStatusFile);
            batStatus = stream.readLine();
        }
        batStatusFile.close();
        if ( batStatus.contains( "Charging",Qt::CaseInsensitive ) ) {
            chargeStatusText = "↗";
        }
        if ( batStatus.contains( "Discharging",Qt::CaseInsensitive ) ) {
            chargeStatusText = "↘";
        }
        // Capacity
        QFile file(BATTERY_CAPACITY_PATH);
        if(!file.exists()){
            mVoltage = "ERR";
            emit voltageValueChanged();
        }
        QString line;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream stream(&file);
            line = stream.readLine();
        }
        file.close();
        mVoltage = "" + line + " % " + chargeStatusText;
        emit voltageValueChanged();
        int voltCompare = line.toInt();
        // Green > 20 %
        if ( voltCompare > 20 ) {
            mvoltageNotifyColor = mMainColor;
            emit voltageNotifyColorChanged();
        }
        // Yellow 10 - 20 %
        if ( voltCompare >= 10 && voltCompare <= 20 ) {
            mvoltageNotifyColor = "#FFFF00";
            emit voltageNotifyColorChanged();
        }
        // Red < 10 %
        if ( voltCompare < 10 ) {
            mvoltageNotifyColor = "#FF5555";
            emit voltageNotifyColorChanged();
        }
    }

    QString networkStatusFile="/tmp/network";
    QFile networkFile(networkStatusFile);
    if(!networkFile.exists()){
        qDebug() << "Error, no file: " << networkStatusFile;
    }
    QString networkMeasurementValues;
    if (networkFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream stream(&networkFile);
        while (!stream.atEnd()){
            networkMeasurementValues = stream.readLine();
        }
    }
    networkFile.close();

    // <Average Latency in μs> <Standard Deviation in μs> <Percentage of Loss>
    QStringList networkElements = networkMeasurementValues.split(' ');
    int latencyIntms = networkElements[0].toInt()/1000;
    QString indicateValue = QString::number(latencyIntms) + " ms";
    mnetworkStatusLabelValue = indicateValue;
    mnetworkStatusLabelColor=mMainColor;
    if ( latencyIntms < 200 ) {
        mnetworkStatusLabelColor=mMainColor;
    }
    if ( latencyIntms == 0 ) {
        mnetworkStatusLabelColor="#FF5555";
    }
    if ( latencyIntms > 200 ) {
        mnetworkStatusLabelColor="#FFFF00";
    }
    if ( latencyIntms > 1000 ) {
        mnetworkStatusLabelColor="#FF5555";
    }
    emit networkStatusLabelChanged();
    emit networkStatusLabelColorChanged();

    /* Screen timeout counter */
    if ( m_screenTimeoutCounter > 0 && m_deviceLocked == false && g_connectState == false ) {
        m_screenTimeoutCounter = m_screenTimeoutCounter - 5;
    }
    if ( m_screenTimeoutCounter == 0 && m_deviceLocked == false ) {
        lockDevice(LOCK_DEVICE);
    }
    peerLatency();

    // Stop shutdown timer if connection is active
    if ( g_connectState && m_automaticShutdownEnabled && automaticShutdownTimer->isActive() ) {
        automaticShutdownTimer->stop();
    }
    // Start shutdown timer if connection is inactive
    if ( !g_connectState && m_automaticShutdownEnabled && !automaticShutdownTimer->isActive() ) {
        automaticShutdownTimer->start( AUTOMATIC_SHUTDOWNTIME );
    }
}

/* Read dpinger service output file for peers */
void engineClass::peerLatency()
{
    QStringList peerStatusFileNames = { "/tmp/peer0","/tmp/peer1","/tmp/peer2","/tmp/peer3","/tmp/peer4", "/tmp/peer5", "/tmp/peer6", "/tmp/peer7", "/tmp/peer8", "/tmp/peer9" };

    for (int i = 0; i < peerStatusFileNames.size(); i++) {
        QString fileName=peerStatusFileNames.at(i).toLocal8Bit().constData();
        QString entryLatency;
        QFile peerLatencyFile( fileName );
        if(!peerLatencyFile.exists()) {
            qDebug() << "No latency file: " << fileName;
        } else {
            QString peerLatencyValue;
            if (peerLatencyFile.open(QIODevice::ReadOnly | QIODevice::Text)){
                QTextStream stream(&peerLatencyFile);
                while (!stream.atEnd()){
                    entryLatency = stream.readLine();
                }
            }
            peerLatencyFile.close();
            QStringList latencyElements = entryLatency.split(' ');
            int latencyIntms = latencyElements[0].toInt()/1000;
            m_peerLatencyValue[i] = QString::number(latencyIntms);
        }
    }
    /* Peer latency */
    if ( m_peerLatencyValue[0].toInt() > 0 ) {
        m_peer_0_CallSignColor = mHighColor;
        emit peer_0_NameColorChanged();
    } else {
        m_peer_0_CallSignColor = mMainColor;
        emit peer_0_NameColorChanged();
    }
    if ( m_peerLatencyValue[1].toInt() > 0 ) {
        m_peer_1_CallSignColor = mHighColor;
        emit peer_1_NameColorChanged();
    } else {
        m_peer_1_CallSignColor = mMainColor;
        emit peer_1_NameColorChanged();
    }
    if ( m_peerLatencyValue[2].toInt() > 0 ) {
        m_peer_2_CallSignColor = mHighColor;
        emit peer_2_NameColorChanged();
    } else {
        m_peer_2_CallSignColor = mMainColor;
        emit peer_2_NameColorChanged();
    }
    if ( m_peerLatencyValue[3].toInt() > 0 ) {
        m_peer_3_CallSignColor = mHighColor;
        emit peer_3_NameColorChanged();
    } else {
        m_peer_3_CallSignColor = mMainColor;
        emit peer_3_NameColorChanged();
    }
    if ( m_peerLatencyValue[4].toInt() > 0 ) {
        m_peer_4_CallSignColor = mHighColor;
        emit peer_4_NameColorChanged();
    } else {
        m_peer_4_CallSignColor = mMainColor;
        emit peer_4_NameColorChanged();
    }
    if ( m_peerLatencyValue[5].toInt() > 0 ) {
        m_peer_5_CallSignColor = mHighColor;
        emit peer_5_NameColorChanged();
    } else {
        m_peer_5_CallSignColor = mMainColor;
        emit peer_5_NameColorChanged();
    }
    if ( m_peerLatencyValue[6].toInt() > 0 ) {
        m_peer_6_CallSignColor = mHighColor;
        emit peer_6_NameColorChanged();
    } else {
        m_peer_6_CallSignColor = mMainColor;
        emit peer_6_NameColorChanged();
    }
    if ( m_peerLatencyValue[7].toInt() > 0 ) {
        m_peer_7_CallSignColor = mHighColor;
        emit peer_7_NameColorChanged();
    } else {
        m_peer_7_CallSignColor = mMainColor;
        emit peer_7_NameColorChanged();
    }
    if ( m_peerLatencyValue[8].toInt() > 0 ) {
        m_peer_8_CallSignColor = mHighColor;
        emit peer_8_NameColorChanged();
    } else {
        m_peer_8_CallSignColor = mMainColor;
        emit peer_8_NameColorChanged();
    }
    if ( m_peerLatencyValue[9].toInt() > 0 ) {
        m_peer_9_CallSignColor = mHighColor;
        emit peer_9_NameColorChanged();
    } else {
        m_peer_9_CallSignColor = mMainColor;
        emit peer_9_NameColorChanged();
    }
}


void engineClass::initEngine()
{
    if ( m_vaultModeActive ) {
        return;
    }

    int myOwnNodeId;
    g_connectState = false;

    // Telemetry FIFO watcher
    watcher = new QFileSystemWatcher();
    watcher->addPath(TELEMETRY_FIFO_OUT);
    QObject::connect(watcher, SIGNAL(fileChanged(QString)), this, SLOT(fifoChanged()));

    // Ping daemon
    fifoWrite("127.0.0.1,daemon_ping");

    // Open telemetry FIFO
    fifoIn.setFileName(TELEMETRY_FIFO_OUT);
    if(!fifoIn.open(QIODevice::ReadOnly | QIODevice::Unbuffered | QIODevice::Text)) {
        qDebug() << "error " << fifoIn.errorString();
    }

    // Init message fifo & watcher
    fifoWrite(nodes.myNodeIp + ",message,init"); // nodes.myNodeIp
    QFileSystemWatcher *msgWatcher = new QFileSystemWatcher();
    msgWatcher->addPath(MESSAGE_RECEIVE_FIFO);
    QObject::connect(msgWatcher, SIGNAL(fileChanged(QString)), this, SLOT(msgFifoChanged()));

    // Open incoming message FIFO
    msgFifoIn.setFileName(MESSAGE_RECEIVE_FIFO);
    if(!msgFifoIn.open(QIODevice::ReadOnly | QIODevice::Unbuffered | QIODevice::Text)) {
        qDebug() << "error" << msgFifoIn.errorString();
    }

    /* Activate contact buttons */
    m_button_0_active = true;
    emit button_0_activeChanged();
    m_button_1_active = true;
    emit button_1_activeChanged();
    m_button_2_active = true;
    emit button_2_activeChanged();
    m_button_3_active = true;
    emit button_3_activeChanged();
    m_button_4_active = true;
    emit button_4_activeChanged();
    m_button_5_active = true;
    emit button_5_activeChanged();
    m_button_6_active = true;
    emit button_6_activeChanged();
    m_button_7_active = true;
    emit button_7_activeChanged();
    m_button_8_active = true;
    emit button_8_activeChanged();
    m_button_9_active = true;
    emit button_9_activeChanged();

    /* Disable my own contact button */
    for (int x=0; x < NODECOUNT; x++ ) {

        if ( nodes.node_name[x].compare( nodes.myNodeName ) == 0 )
            myOwnNodeId = x;

         if ( myOwnNodeId == x ) {
             if ( x == 0 ) {
                 m_button_0_active = false;
                 emit button_0_activeChanged();
             }
             if ( x == 1 ) {
                 m_button_1_active = false;
                 emit button_1_activeChanged();
             }
             if ( x == 2 ) {
                 m_button_2_active = false;
                 emit button_2_activeChanged();
             }
             if ( x == 3 )  {
                 m_button_3_active = false;
                 emit button_3_activeChanged();
             }
             if ( x == 4 )  {
                 m_button_4_active = false;
                 emit button_4_activeChanged();
             }
             if ( x == 5 ) {
                 m_button_5_active = false;
                 emit button_5_activeChanged();
             }
             if ( x == 6 ) {
                 m_button_6_active = false;
                 emit button_6_activeChanged();
             }
             if ( x == 7 ) {
                 m_button_7_active = false;
                 emit button_7_activeChanged();
             }
             if ( x == 8 ) {
                 m_button_8_active = false;
                 emit button_8_activeChanged();
             }
             if ( x == 9 ) {
                 m_button_9_active = false;
                 emit button_9_activeChanged();
             }
         }
    }
    m_statusMessage = "Ready!";
    emit statusMessageChanged();
}

/* Msg quick buttons */

void engineClass::quickButtonSend(int sendCode)
{
    if ( g_connectState ) {
        if ( sendCode == 1 )
        {
            QString fifo_command = g_remoteOtpPeerIp + ",message,Ping";
            fifoWrite(fifo_command);
        }
        if ( sendCode == 2 )
        {
            QString fifo_command = g_remoteOtpPeerIp + ",message,Ledredon";
            fifoWrite(fifo_command);
        }
        if ( sendCode == 3 )
        {
            QString fifo_command = g_remoteOtpPeerIp + ",message,Ledredoff";
            fifoWrite(fifo_command);
        }
        if ( sendCode == 4 )
        {
            QString fifo_command = g_remoteOtpPeerIp + ",message,Ledgreenon";
            fifoWrite(fifo_command);
        }
        if ( sendCode == 5 )
        {
            QString fifo_command = g_remoteOtpPeerIp + ",message,Ledgreenoff";
            fifoWrite(fifo_command);
        }
        if ( sendCode == 6 )
        {
            QString fifo_command = g_remoteOtpPeerIp + ",message,SonarPing";
            fifoWrite(fifo_command);
        }
    }
}

/* Messaging fifo handler [pine] */
int engineClass::msgFifoChanged()
{
    QTextStream in(&msgFifoIn);
    QString line = in.readAll();
    QStringList token = line.split(',');

    /* macsec (WiP) */
    QString secondToken = token[1];
    if ( secondToken.contains("macsec_keyed") ) {
        mMacsecKeyed = "LOADED";
        emit macsecKeyedChanged();
        mMacsecKeyValid = true;
        emit macsecValidChanged();
        return 0;
    }
    if ( secondToken.contains("hsm_insert") ) {
        mMacsecKeyed = "KEYING";
        emit macsecKeyedChanged();
        return 0;
    }

    /* Indicate incoming audio request ('ring') */
    if ( token[1] == "ring" )
    {
       if ( m_deviceLocked == true ) {
           lockDevice(UNLOCK_DEVICE);
       }
       m_callDialogVisible = true;
       emit callDialogVisibleChanged();
       runExternalCmd("/bin/pptk-vibrate", {"400","90","2"});
       token[1]="";
       return 0;
    }

       if ( token[1] == "remote_hangup") {
           updateCallStatusIndicator("Remote hangup", "green", "transparent",LOG_AND_INDICATE);
           token[1]="";
           eraseConnectionLabels();
           m_callSignInsigniaImage = "";
           m_insigniaLabelText = "";
           m_insigniaLabelStateText = "";
           emit callSignInsigniaImageChanged();
           emit insigniaLabelTextChanged();
           emit insigniaLabelStateTextChanged();
           disconnectButton();
           return 0;
       }

       if ( token[1] == "answer_success") {
           updateCallStatusIndicator("Audio active", "lightgreen", "transparent",INDICATE_ONLY);
           token[1]="";
           mAudioDeviceBusy = true;
           return 0;
       }

       // Remote (who connected us) press 'terminate', we should do the same
       if ( token[1] == "initiator_disconnect") {
           // Hangup to FIFO
           QString hangupCommandString = g_connectedNodeIp + ",hangup";
           fifoWrite( hangupCommandString );
           // Wait FIFO with timeout
           if ( waitForFifoReply() == FIFO_TIMEOUT ) {
               updateCallStatusIndicator("Timeout. Aborting.", "green", "transparent",LOG_ONLY );
               return 0;
           }
           // Turn off local audio
           hangupCommandString = "127.0.0.1,disconnect_audio";
           fifoWrite( hangupCommandString );
           updateCallStatusIndicator("Incoming Terminated", "lightgreen", "transparent",INDICATE_ONLY);
           eraseConnectionLabels();
           if ( m_messageEraseEnabled ) {
               m_textMsgDisplay = "";
               emit textMsgDisplayChanged();
           }
           g_connectState = false;
           m_callSignInsigniaImage = "";
           m_insigniaLabelText = "";
           m_insigniaLabelStateText = "";
           m_goSecureButton_active = false;
           emit callSignInsigniaImageChanged();
           emit insigniaLabelTextChanged();
           emit insigniaLabelStateTextChanged();
           emit goSecureButton_activeChanged();
           m_callDialogVisible = false;
           emit callDialogVisibleChanged();
           // Return to main page
           m_SwipeViewIndex = 0;
           emit swipeViewIndexChanged();
           token[1]="";
           mAudioDeviceBusy = false;
           return 0;
       }

       // client_connected,[client_id];[client_ip];[client_name]
       if ( token[1].contains( "client_connected",Qt::CaseInsensitive ) ) {
           qDebug() << "** Remote client connected **";
           if ( m_deviceLocked ) {
               lockDevice(UNLOCK_DEVICE);
           }
           QStringList remoteParameters = token[1].split(';');
           g_connectedNodeId = remoteParameters[1];
           g_connectedNodeIp = remoteParameters[2];
           // remote name: remoteParameters[3]
           g_connectState = true;
           updateCallStatusIndicator(remoteParameters[3] + " connected" , "lightgreen","transparent",LOG_AND_INDICATE);
           token[1]="";

           // Light up green label for connected name
           setIndicatorForIncomingConnection(remoteParameters[2]);

           // Search ID for connectedNodeId and activate insignia
           int insigniaNodeId=-1;
           for (int x=0; x < NODECOUNT; x++ ) {
               if ( nodes.node_id[x].compare( g_connectedNodeId ) == 0 )
                   insigniaNodeId = x;
           }
           if ( insigniaNodeId != -1 )
               activateInsignia(insigniaNodeId, "Incoming connection");

           // Disable 'Go Secure' TODO: and Contacts until terminate
           m_goSecureButton_active = false;
           emit goSecureButton_activeChanged();

           // Inbound OTP, remote is 10.10.0.2 (client) and I am 10.10.0.1 (server)
           g_remoteOtpPeerIp = "10.10.0.2";

           // Erase messaging
           if ( m_messageEraseEnabled ) {
               m_textMsgDisplay = "";
               emit textMsgDisplayChanged();
           }
           return 0;
       }

       /* ping - pong */
       if ( token[1] == "Ping") {
           if ( g_connectState ) {
               QString fifo_command = g_remoteOtpPeerIp + ",message,CommCheck [ " + mVoltage + " ] [ " + mnetworkStatusLabelValue +" ]";
               fifoWrite(fifo_command);
               return 0;
           }
       }
       /* Red Led Setting */
       if ( token[1] == "Ledredon") {
           if ( g_connectState ) {
               runExternalCmd("/bin/pptk-led", {"set", "red", "1"});
               QString fifo_command = g_remoteOtpPeerIp + ",message,Red led ON [ " + mVoltage + " ] [ " + mnetworkStatusLabelValue +" ]";
               fifoWrite(fifo_command);
               return 0;
           }
       }
       if ( token[1] == "Ledredoff") {
           if ( g_connectState ) {
               runExternalCmd("/bin/pptk-led", {"set", "red", "0"});
               QString fifo_command = g_remoteOtpPeerIp + ",message,Red led OFF [ " + mVoltage + " ] [ " + mnetworkStatusLabelValue +" ]";
               fifoWrite(fifo_command);
               return 0;
           }
       }
       /* Green Led Setting */
       if ( token[1] == "Ledgreenon") {
           if ( g_connectState ) {
               runExternalCmd("/bin/pptk-led", {"set", "green", "1"});
               QString fifo_command = g_remoteOtpPeerIp + ",message,Green led ON [ " + mVoltage + " ] [ " + mnetworkStatusLabelValue +" ]";
               fifoWrite(fifo_command);
               return 0;
           }
       }
       if ( token[1] == "Ledgreenoff") {
           if ( g_connectState ) {
               runExternalCmd("/bin/pptk-led", {"set", "green", "0"});
               QString fifo_command = g_remoteOtpPeerIp + ",message,Green led OFF [ " + mVoltage + " ] [ " + mnetworkStatusLabelValue +" ]";
               fifoWrite(fifo_command);
               return 0;
           }
       }
       if ( token[1] == "SonarPing") {
           if ( g_connectState ) {
               if ( !mAudioDeviceBusy ) {
                   runExternalCmd("/bin/aplay", {"/etc/sonar.wav"});
                   QString fifo_command = g_remoteOtpPeerIp + ",message,Sonar ping played [ " + mVoltage + " ] [ " + mnetworkStatusLabelValue +" ]";
                   fifoWrite(fifo_command);
               }
               return 0;
           }
       }
       /* Normal message to be shown. */
       if (token[1] != "" )
       {
           if ( g_connectState == true ) {
               if ( m_deviceLocked == true ) {
                   qDebug() << "Message received in locked mode";
                   lockDevice(UNLOCK_DEVICE);
               }
               /* Swipe only unlocked */
               if ( m_lockScreen_active == false ) {
                   qDebug() << "Message received in m_lockScreen_active == false";
                   m_SwipeViewIndex = 1;
                   emit swipeViewIndexChanged();
               }
               /* Vibrate test */
               runExternalCmd("/bin/pptk-vibrate", {"200","200","1"});
               /* Display message */
               token[1].replace( QChar(SUBSTITUTE_CHAR_CODE), "," );
               m_textMsgDisplay = m_textMsgDisplay + "<br> <font color='" + mMessageColorRemote + "'>" + token[1] + "</font>";
               emit textMsgDisplayChanged();
               return 0;
           }
       }
       return 0;
}

void engineClass::eraseConnectionLabels()
{
    // Erase green status
    m_peer_0_connection_label = false;
    m_peer_1_connection_label = false;
    m_peer_2_connection_label = false;
    m_peer_3_connection_label = false;
    m_peer_4_connection_label = false;
    m_peer_5_connection_label = false;
    m_peer_6_connection_label = false;
    m_peer_7_connection_label = false;
    m_peer_8_connection_label = false;
    m_peer_9_connection_label = false;
    emit peer_0_LabelChanged();
    emit peer_1_LabelChanged();
    emit peer_2_LabelChanged();
    emit peer_3_LabelChanged();
    emit peer_4_LabelChanged();
    emit peer_5_LabelChanged();
    emit peer_6_LabelChanged();
    emit peer_7_LabelChanged();
    emit peer_8_LabelChanged();
    emit peer_9_LabelChanged();
}

/* Incoming connection indicates who connected
    TODO: Disable contact buttons when incoming connection is active?
*/
void engineClass::setIndicatorForIncomingConnection(QString peerIp)
{
   eraseConnectionLabels();

    int nodeNumber;
    for (int x=0; x<NODECOUNT; x++) {
      if(peerIp.compare(nodes.node_ip[x]) == 0) {
          nodeNumber=x;
      }
    }
    if( nodeNumber == 0 ) {
          m_peer_0_connection_label = true;
          emit peer_0_LabelChanged();
          m_peer_0_connection_label_color = mMainColor;
          emit peer_0_LabelColorChanged();
    }
    if( nodeNumber == 1 ) {
          m_peer_1_connection_label = true;
          emit peer_1_LabelChanged();
          m_peer_1_connection_label_color = mMainColor;
          emit peer_1_LabelColorChanged();
    }
    if( nodeNumber == 2 ) {
          m_peer_2_connection_label = true;
          emit peer_2_LabelChanged();
          m_peer_2_connection_label_color = mMainColor;
          emit peer_2_LabelColorChanged();
    }
    if( nodeNumber == 3 ) {
          m_peer_3_connection_label = true;
          emit peer_3_LabelChanged();
          m_peer_3_connection_label_color = mMainColor;
          emit peer_3_LabelColorChanged();
    }
    if( nodeNumber == 4 ) {
          m_peer_4_connection_label = true;
          emit peer_4_LabelChanged();
          m_peer_4_connection_label_color = mMainColor;
          emit peer_4_LabelColorChanged();
    }
    if( nodeNumber == 5 ) {
          m_peer_5_connection_label = true;
          emit peer_5_LabelChanged();
          m_peer_5_connection_label_color = mMainColor;
          emit peer_5_LabelColorChanged();
    }
    if( nodeNumber == 6 ) {
          m_peer_6_connection_label = true;
          emit peer_6_LabelChanged();
          m_peer_6_connection_label_color = mMainColor;
          emit peer_6_LabelColorChanged();
    }
    if( nodeNumber == 7 ) {
          m_peer_7_connection_label = true;
          emit peer_7_LabelChanged();
          m_peer_7_connection_label_color = mMainColor;
          emit peer_7_LabelColorChanged();
    }
    if( nodeNumber == 8 ) {
          m_peer_8_connection_label = true;
          emit peer_8_LabelChanged();
          m_peer_8_connection_label_color = mMainColor;
          emit peer_8_LabelColorChanged();
    }
    if( nodeNumber == 9 ) {
          m_peer_9_connection_label = true;
          emit peer_9_LabelChanged();
          m_peer_9_connection_label_color = mMainColor;
          emit peer_9_LabelColorChanged();
    }
}

void engineClass::fifoWrite(QString message)
{
    if ( fifoIn.isOpen() ) {
        QTextStream in(&fifoIn);        // dummy read
        QString line = in.readAll();    // dummy read
    }
    g_fifoReply = "";
    QFile file(TELEMETRY_FIFO_IN);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "FIFO Write file open error" << file.errorString();
    }
    QTextStream out(&file);
    out << message << Qt::endl;
    file.close();
}


/* Telemetry FIFO [PINE] */
int engineClass::fifoChanged()
{
    int nodeNumber;
    QTextStream in(&fifoIn);
    QString line = in.readAll();

    if ( line.length() == 0 ) {
        qDebug() << "EMPTY FIFO Received";
        return 0;
    }
    if(line.compare("telemetryclient_is_alive") == 0) {
          g_fifoReply = "client_alive";
    } else {

        /* Main logic for telemetry fifo handling */
        QStringList token = line.split(',');
        g_fifoReply = token[1];
        // qDebug() << "Telemetry FIFO received:  IP:" << token[0] << " Status: " << token[1];
          if( token[1].compare("available") == 0 )
          {
              for (int x=0; x<NODECOUNT; x++) {
                if(token[0].compare(nodes.node_ip[x]) == 0) {
                    nodeNumber=x;
                }
              }
              if( nodeNumber == 0 ) {
                    connectAsClient(nodes.node_ip[nodeNumber], nodes.node_id[nodeNumber]);
                    m_peer_0_connection_label = true;
                    emit peer_0_LabelChanged();
                    m_peer_0_connection_label_color = mMainColor;
                    emit peer_0_LabelColorChanged();
              }
              if( nodeNumber == 1 ) {
                    connectAsClient(nodes.node_ip[nodeNumber], nodes.node_id[nodeNumber]);
                    m_peer_1_connection_label = true;
                    emit peer_1_LabelChanged();
                    m_peer_1_connection_label_color = mMainColor;
                    emit peer_1_LabelColorChanged();
              }
              if( nodeNumber == 2 ) {
                    connectAsClient(nodes.node_ip[nodeNumber], nodes.node_id[nodeNumber]);
                    m_peer_2_connection_label = true;
                    emit peer_2_LabelChanged();
                    m_peer_2_connection_label_color = mMainColor;
                    emit peer_2_LabelColorChanged();
              }
              if( nodeNumber == 3 ) {
                    connectAsClient(nodes.node_ip[nodeNumber], nodes.node_id[nodeNumber]);
                    m_peer_3_connection_label = true;
                    emit peer_3_LabelChanged();
                    m_peer_3_connection_label_color = mMainColor;
                    emit peer_3_LabelColorChanged();
              }
              if( nodeNumber == 4 ) {
                    connectAsClient(nodes.node_ip[nodeNumber], nodes.node_id[nodeNumber]);
                    m_peer_4_connection_label = true;
                    emit peer_4_LabelChanged();
                    m_peer_4_connection_label_color = mMainColor;
                    emit peer_4_LabelColorChanged();
              }
              if( nodeNumber == 5 ) {
                    connectAsClient(nodes.node_ip[nodeNumber], nodes.node_id[nodeNumber]);
                    m_peer_5_connection_label = true;
                    emit peer_5_LabelChanged();
                    m_peer_5_connection_label_color = mMainColor;
                    emit peer_5_LabelColorChanged();
              }
              if( nodeNumber == 6 ) {
                    connectAsClient(nodes.node_ip[nodeNumber], nodes.node_id[nodeNumber]);
                    m_peer_6_connection_label = true;
                    emit peer_6_LabelChanged();
                    m_peer_6_connection_label_color = mMainColor;
                    emit peer_6_LabelColorChanged();
              }
              if( nodeNumber == 7 ) {
                    connectAsClient(nodes.node_ip[nodeNumber], nodes.node_id[nodeNumber]);
                    m_peer_7_connection_label = true;
                    emit peer_7_LabelChanged();
                    m_peer_7_connection_label_color = mMainColor;
                    emit peer_7_LabelColorChanged();
              }
              if( nodeNumber == 8 ) {
                    connectAsClient(nodes.node_ip[nodeNumber], nodes.node_id[nodeNumber]);
                    m_peer_8_connection_label = true;
                    emit peer_8_LabelChanged();
                    m_peer_8_connection_label_color = mMainColor;
                    emit peer_8_LabelColorChanged();
              }
              if( nodeNumber == 9 ) {
                    connectAsClient(nodes.node_ip[nodeNumber], nodes.node_id[nodeNumber]);
                    m_peer_9_connection_label = true;
                    emit peer_9_LabelChanged();
                    m_peer_9_connection_label_color = mMainColor;
                    emit peer_9_LabelColorChanged();
              }
          }
          // TODO: Terminate should erase 'red ones'
          if( token[1].compare("offline") == 0 )
          {
              for (int x=0; x<NODECOUNT; x++) {
                if(token[0].compare(nodes.node_ip[x]) == 0) {
                    nodeNumber=x;
                }
              }
              if( nodeNumber == 0 ) {
                  m_peer_0_connection_label = true;
                  emit peer_0_LabelChanged();
                  m_peer_0_connection_label_color = "red";
                  emit peer_0_LabelColorChanged();
              }
              if( nodeNumber == 1 ) {
                  m_peer_1_connection_label = true;
                  emit peer_1_LabelChanged();
                  m_peer_1_connection_label_color = "red";
                  emit peer_1_LabelColorChanged();
              }
              if( nodeNumber == 2 ) {
                  m_peer_2_connection_label = true;
                  emit peer_2_LabelChanged();
                  m_peer_2_connection_label_color = "red";
                  emit peer_2_LabelColorChanged();
              }
              if( nodeNumber == 3 ) {
                  m_peer_3_connection_label = true;
                  emit peer_3_LabelChanged();
                  m_peer_3_connection_label_color = "red";
                  emit peer_3_LabelColorChanged();
              }
              if( nodeNumber == 4 ) {
                  m_peer_4_connection_label = true;
                  emit peer_4_LabelChanged();
                  m_peer_4_connection_label_color = "red";
                  emit peer_4_LabelColorChanged();
              }
              if( nodeNumber == 5 ) {
                  m_peer_5_connection_label = true;
                  emit peer_5_LabelChanged();
                  m_peer_5_connection_label_color = "red";
                  emit peer_5_LabelColorChanged();
              }
              if( nodeNumber == 6 ) {
                  m_peer_6_connection_label = true;
                  emit peer_6_LabelChanged();
                  m_peer_6_connection_label_color = "red";
                  emit peer_6_LabelColorChanged();
              }
              if( nodeNumber == 7 ) {
                  m_peer_7_connection_label = true;
                  emit peer_7_LabelChanged();
                  m_peer_7_connection_label_color = "red";
                  emit peer_7_LabelColorChanged();
              }
              if( nodeNumber == 8 ) {
                  m_peer_8_connection_label = true;
                  emit peer_8_LabelChanged();
                  m_peer_8_connection_label_color = "red";
                  emit peer_8_LabelColorChanged();
              }
              if( nodeNumber == 9 ) {
                  m_peer_9_connection_label = true;
                  emit peer_9_LabelChanged();
                  m_peer_9_connection_label_color = "red";
                  emit peer_9_LabelColorChanged();
              }
          }

          if( token[1].compare("terminate_ready") == 0 )
          {
              updateCallStatusIndicator("OTP disconnected", "green", "transparent",INDICATE_ONLY );
              eraseConnectionLabels();
              m_callSignInsigniaImage = "";
              m_insigniaLabelText = "";
              m_insigniaLabelStateText = "";
              emit callSignInsigniaImageChanged();
              emit insigniaLabelTextChanged();
              emit insigniaLabelStateTextChanged();
              m_goSecureButton_active = false;
              emit goSecureButton_activeChanged();
          }
          if( token[1].compare("busy") == 0 )
          {
              updateCallStatusIndicator("Remote busy", "green", "transparent",LOG_ONLY );
          }
          if( token[1].compare("offline") == 0 )
          {
              updateCallStatusIndicator("Remote offline", "green", "transparent",LOG_ONLY );
          }
      }

    /* Other status codes (TODO):
        busy
        terminate_ready
        prepare_ready
        ring_ready
    */
    return 0;
}

/* Connect as client ('initiator') to peer ('as OTP server') */
void engineClass::connectAsClient(QString nodeIp, QString nodeId)
{
    // 1. Send 'prepare' to recipient via FIFO
    QString prepareFifoCmd = nodeIp + ",prepare";
    fifoWrite(prepareFifoCmd);

    // Wait FIFO with timeout
    if ( waitForFifoReply() == FIFO_TIMEOUT ) {
        updateCallStatusIndicator("Timeout. Aborting.", "green", "transparent",LOG_ONLY );
        return;
    }

    updateCallStatusIndicator("Remote prepared", "black","yellow",LOG_AND_INDICATE);

    // 2. Start local service for OTP to targeted node as 'client' role
    QString serviceNameAsClient = "connect-with-"+nodeId+"-c.service";
    qint64 pid;
    QProcess process;
    process.setProgram("systemctl");
    process.setArguments({"start",serviceNameAsClient});
    process.startDetached(&pid);

    // 3. Touch local file
    touchLocalFile("/tmp/CLIENT_CALL_ACTIVE");

    // Now we should have OTP connectivity ready
    g_connectState = true;
    g_connectedNodeId = nodeId;
    g_connectedNodeIp = nodeIp;

    // 4. Indicate OTP connected
    updateCallStatusIndicator("OTP connected", "lightgreen", "transparent",INDICATE_ONLY);

    // Update insignia status text
    m_insigniaLabelStateText = "Connected";
    emit insigniaLabelStateTextChanged();

    // Erase messaging history
    if ( m_messageEraseEnabled ) {
        m_textMsgDisplay = "";
        emit textMsgDisplayChanged();
    }

    // 4.5 We should disable Peer keys when connected (TODO)
    // setContactButtons(false);

    // Enable 'Go Secure' when connection is made as Client role
    m_goSecureButton_active = true;
    emit goSecureButton_activeChanged();

    // 5. Set fixed remote IP based on client role of connection (and client is 10.10.0.2)
    g_remoteOtpPeerIp = "10.10.0.1";

    // TODO: This is utilized when MSG's are sent over OTP channel. (work in progress)

    // 6. Indicate remote peer UI that we're connected WORK IN PROGRESS!!
    QString informRemoteUi = nodeIp + ",message,client_connected;"+nodes.myNodeId+";"+nodes.myNodeIp+";"+nodes.myNodeName;
    fifoWrite(informRemoteUi);

    // Wait FIFO with timeout
    if ( waitForFifoReply() == FIFO_TIMEOUT ) {
        updateCallStatusIndicator("Timeout. Aborting.", "green", "transparent",LOG_ONLY );
        return;
    }

    // 7. Audio gets established by remote end sending 'answer'
    //    after 'Go Secure' is pressed. So it's actually remote peer
    //    who activates audio on this calling 'client' side (!)

    // 8. After termination => terminate_ready

}

/* Disconnect from 'client' side */
void engineClass::disconnectAsClient(QString nodeIp, QString nodeId)
{
    // 1. terminate to FIFO
    QString terminateFifoCmd = nodeIp + ",terminate";
    fifoWrite(terminateFifoCmd);

    // Wait FIFO with timeout
    if ( waitForFifoReply() == FIFO_TIMEOUT ) {
        updateCallStatusIndicator("Timeout. Aborting.", "green", "transparent",LOG_ONLY );
        return;
    }

    // 2a. stop local service for targeted node (as client)
    QString serviceNameAsClient = "connect-with-"+nodeId+"-c.service";
    qint64 pid;
    QProcess process;
    process.setProgram("systemctl");
    process.setArguments({"stop",serviceNameAsClient});
    process.startDetached(&pid);

    // 2b. stop local service for targeted node (as server)
    QString serviceNameAsServer = "connect-with-"+nodeId+"-s.service";
    qint64 s_pid;
    QProcess s_process;
    s_process.setProgram("systemctl");
    s_process.setArguments({"stop",serviceNameAsServer});
    s_process.startDetached(&s_pid);

    // 3. Remove status file
    removeLocalFile("/tmp/CLIENT_CALL_ACTIVE");

    // 4 Send UI message to remote for disconnect indications for remote UI
    QString informRemoteUi = nodeIp + ",message,remote_hangup";
    fifoWrite(informRemoteUi);

    // Wait FIFO with timeout
    if ( waitForFifoReply() == FIFO_TIMEOUT ) {
        updateCallStatusIndicator("Timeout. Aborting.", "green", "transparent",LOG_ONLY );
        return;
    }

    // 5. Terminate local audio (TODO)
    // 'telemetryclient' knows how to terminate audio, based on how it's established (client or server)
    QString terminateAudioFifoCmd = "127.0.0.1,disconnect_audio";
    fifoWrite(terminateAudioFifoCmd);
    g_connectState = false;
    g_connectedNodeId = "";
    g_connectedNodeIp = "";
    g_remoteOtpPeerIp = "";

}

/* Go Secure button clicked ('Call') */
void engineClass::on_goSecure_clicked()
{
    updateCallStatusIndicator("Waiting remote", "lightgreen","transparent",INDICATE_ONLY);
    // This is shell script ring -> ring_ready
    QString callString = g_connectedNodeIp + ",ring";
    fifoWrite( callString );
    // Wait FIFO with timeout
    if ( waitForFifoReply() == FIFO_TIMEOUT ) {
        updateCallStatusIndicator("Timeout. Aborting.", "green", "transparent",LOG_ONLY );
        return;
    }
    // Ring also on UI
    callString = g_connectedNodeIp + ",message,ring";
    fifoWrite( callString );
}

// NOTE: Pine version does not use (yet) method & colors !!
void engineClass::updateCallStatusIndicator(QString text, QString fontColor, QString backgroundColor, int logMethod )
{
    m_statusMessage = text;
    emit statusMessageChanged();
}
void engineClass::touchLocalFile(QString filename)
{
    QFile touchFile(filename);
    touchFile.open( QIODevice::WriteOnly);
    touchFile.close();
}
void engineClass::removeLocalFile(QString filename)
{
    QFile file (filename);
    file.remove();
}

/* Sample debug from QML side: eClass.debugThis('string') */
void engineClass::debugThis(QString debugMessage)
{
    qDebug() << "Engine class debug: " << debugMessage;

}

void engineClass::powerOff()
{
    qint64 pid;
    QProcess process;
    process.setProgram("poweroff");
    process.setArguments({"-f"});
    process.startDetached(&pid);
}

QString engineClass::getPeer_0_Name()
{
    return m_peer_0_CallSign;
}
QString engineClass::getPeer_1_Name()
{
    return m_peer_1_CallSign;
}
QString engineClass::getPeer_2_Name()
{
    return m_peer_2_CallSign;
}
QString engineClass::getPeer_3_Name()
{
    return m_peer_3_CallSign;
}
QString engineClass::getPeer_4_Name()
{
    return m_peer_4_CallSign;
}
QString engineClass::getPeer_5_Name()
{
    return m_peer_5_CallSign;
}
QString engineClass::getPeer_6_Name()
{
    return m_peer_6_CallSign;
}
QString engineClass::getPeer_7_Name()
{
    return m_peer_7_CallSign;
}
QString engineClass::getPeer_8_Name()
{
    return m_peer_8_CallSign;
}
QString engineClass::getPeer_9_Name()
{
    return m_peer_9_CallSign;
}


QString engineClass::getPeer_0_NameColor()
{
    return m_peer_0_CallSignColor;
}
QString engineClass::getPeer_1_NameColor()
{
    return m_peer_1_CallSignColor;
}
QString engineClass::getPeer_2_NameColor()
{
    return m_peer_2_CallSignColor;
}
QString engineClass::getPeer_3_NameColor()
{
    return m_peer_3_CallSignColor;
}
QString engineClass::getPeer_4_NameColor()
{
    return m_peer_4_CallSignColor;
}
QString engineClass::getPeer_5_NameColor()
{
    return m_peer_5_CallSignColor;
}
QString engineClass::getPeer_6_NameColor()
{
    return m_peer_6_CallSignColor;
}
QString engineClass::getPeer_7_NameColor()
{
    return m_peer_7_CallSignColor;
}
QString engineClass::getPeer_8_NameColor()
{
    return m_peer_8_CallSignColor;
}
QString engineClass::getPeer_9_NameColor()
{
    return m_peer_9_CallSignColor;
}


bool engineClass::getPeer_0_Label()
{
    return m_peer_0_connection_label;
}

bool engineClass::getPeer_1_Label()
{
    return m_peer_1_connection_label;
}
bool engineClass::getPeer_2_Label()
{
    return m_peer_2_connection_label;
}
bool engineClass::getPeer_3_Label()
{
    return m_peer_3_connection_label;
}
bool engineClass::getPeer_4_Label()
{
    return m_peer_4_connection_label;
}
bool engineClass::getPeer_5_Label()
{
    return m_peer_5_connection_label;
}
bool engineClass::getPeer_6_Label()
{
    return m_peer_6_connection_label;
}
bool engineClass::getPeer_7_Label()
{
    return m_peer_7_connection_label;
}
bool engineClass::getPeer_8_Label()
{
    return m_peer_8_connection_label;
}
bool engineClass::getPeer_9_Label()
{
    return m_peer_9_connection_label;
}


QString engineClass::getPeer_0_Label_color()
{
    return m_peer_0_connection_label_color;
}
QString engineClass::getPeer_1_Label_color()
{
    return m_peer_1_connection_label_color;
}
QString engineClass::getPeer_2_Label_color()
{
    return m_peer_2_connection_label_color;
}
QString engineClass::getPeer_3_Label_color()
{
    return m_peer_3_connection_label_color;
}
QString engineClass::getPeer_4_Label_color()
{
    return m_peer_4_connection_label_color;
}
QString engineClass::getPeer_5_Label_color()
{
    return m_peer_5_connection_label_color;
}
QString engineClass::getPeer_6_Label_color()
{
    return m_peer_6_connection_label_color;
}
QString engineClass::getPeer_7_Label_color()
{
    return m_peer_7_connection_label_color;
}
QString engineClass::getPeer_8_Label_color()
{
    return m_peer_8_connection_label_color;
}
QString engineClass::getPeer_9_Label_color()
{
    return m_peer_9_connection_label_color;
}


bool engineClass::getButton_0_active()
{
    return m_button_0_active;
}
bool engineClass::getButton_1_active()
{
    return m_button_1_active;
}
bool engineClass::getButton_2_active()
{
    return m_button_2_active;
}
bool engineClass::getButton_3_active()
{
    return m_button_3_active;
}
bool engineClass::getButton_4_active()
{
    return m_button_4_active;
}
bool engineClass::getButton_5_active()
{
    return m_button_5_active;
}
bool engineClass::getButton_6_active()
{
    return m_button_6_active;
}
bool engineClass::getButton_7_active()
{
    return m_button_7_active;
}
bool engineClass::getButton_8_active()
{
    return m_button_8_active;
}
bool engineClass::getButton_9_active()
{
    return m_button_9_active;
}

QString engineClass::getPeer_0_keyPercentage()
{
    QString keyValueString="";
    if ( m_keyPersentage_incount[0] != "" ) {
        keyValueString=m_keyPersentage_incount[0] + "/" + m_keyPersentage_outcount[0];
    }
    return keyValueString;
}

QString engineClass::getPeer_1_keyPercentage()
{
    QString keyValueString="";
    if ( m_keyPersentage_incount[1] != "" ) {
        keyValueString=m_keyPersentage_incount[1] + "/" + m_keyPersentage_outcount[1];
    }
    return keyValueString;
}
QString engineClass::getPeer_2_keyPercentage()
{
    QString keyValueString="";
    if ( m_keyPersentage_incount[2] != "" ) {
        keyValueString=m_keyPersentage_incount[2] + "/" + m_keyPersentage_outcount[2];
    }
    return keyValueString;
}
QString engineClass::getPeer_3_keyPercentage()
{
    QString keyValueString="";
    if ( m_keyPersentage_incount[3] != "" ) {
        keyValueString=m_keyPersentage_incount[3] + "/" + m_keyPersentage_outcount[3];
    }
    return keyValueString;
}
QString engineClass::getPeer_4_keyPercentage()
{
    QString keyValueString="";
    if ( m_keyPersentage_incount[4] != "" ) {
        keyValueString=m_keyPersentage_incount[4] + "/" + m_keyPersentage_outcount[4];
    }
    return keyValueString;
}
QString engineClass::getPeer_5_keyPercentage()
{
    QString keyValueString="";
    if ( m_keyPersentage_incount[5] != "" ) {
        keyValueString=m_keyPersentage_incount[5] + "/" + m_keyPersentage_outcount[5];
    }
    return keyValueString;
}

QString engineClass::getPeer_6_keyPercentage()
{
    QString keyValueString="";
    if ( m_keyPersentage_incount[6] != "" ) {
        keyValueString=m_keyPersentage_incount[6] + "/" + m_keyPersentage_outcount[6];
    }
    return keyValueString;
}
QString engineClass::getPeer_7_keyPercentage()
{
    QString keyValueString="";
    if ( m_keyPersentage_incount[7] != "" ) {
        keyValueString=m_keyPersentage_incount[7] + "/" + m_keyPersentage_outcount[7];
    }
    return keyValueString;
}
QString engineClass::getPeer_8_keyPercentage()
{
    QString keyValueString="";
    if ( m_keyPersentage_incount[8] != "" ) {
        keyValueString=m_keyPersentage_incount[8] + "/" + m_keyPersentage_outcount[8];
    }
    return keyValueString;
}
QString engineClass::getPeer_9_keyPercentage()
{
    QString keyValueString="";
    if ( m_keyPersentage_incount[9] != "" ) {
        keyValueString=m_keyPersentage_incount[9] + "/" + m_keyPersentage_outcount[9];
    }
    return keyValueString;
}


bool engineClass::getTouchBlock_active()
{
    return m_touchBlock_active;
}

bool engineClass::getLockScreen_active()
{
    return m_lockScreen_active;
}
bool engineClass::getCamoScreen_active()
{
    return m_camoScreen_active;
}

bool engineClass::getVaultScreen_active()
{
    return m_vaultModeActive;
}

QString engineClass::getVaultScreenNotifyText()
{
    return m_vaultNotifyText;
}

QString engineClass::getVaultScreenNotifyColor()
{
    return m_vaultNotifyColor;
}

QString engineClass::getVaultScreenNotifyTextColor()
{
    return m_vaultNotifyTextColor;
}


// START
void engineClass::onVaultProcessReadyReadStdOutput()
{
    vaultOpenProcess.setReadChannel(QProcess::StandardOutput);
    QTextStream stream(&vaultOpenProcess);
    while (!stream.atEnd()) {
       QString line = stream.readLine();
       m_vaultNotifyText = "SUCCESS";
       m_vaultNotifyColor = "green";
       m_vaultNotifyTextColor = "white";
       m_busyIndicatorActive = false;
       emit vaultScreenNotifyTextChanged();
       emit vaultScreenNotifyColorChanged();
       emit vaultScreenNotifyTextColorChanged();
       emit busyIndicatorChanged();
       QTimer::singleShot(4 * 1000, this, SLOT(exitVaultOpenProcess()));
    }
}
void engineClass::exitVaultOpenProcess()
{
    QCoreApplication::quit();
}

void engineClass::exitVaultOpenProcessWithFail()
{
    m_lockScreenPinCode="";
    m_vaultNotifyText = "SELECT VAULT & INPUT PIN";
    m_vaultNotifyColor = "red";
    m_vaultNotifyTextColor = "white";
    emit lockScreenPinCodeChanged();
    emit vaultScreenNotifyTextChanged();
    emit vaultScreenNotifyColorChanged();
    emit vaultScreenNotifyTextColorChanged();
}

void engineClass::onVaultProcessFinished()
{
    QString perr = vaultOpenProcess.readAllStandardError();
    if (perr.length()) {
        m_vaultNotifyText = "FAILED";
        m_vaultNotifyColor = "red";
        m_vaultNotifyTextColor = "white";
        m_busyIndicatorActive = false;
        emit vaultScreenNotifyTextChanged();
        emit vaultScreenNotifyColorChanged();
        emit vaultScreenNotifyTextColorChanged();
        emit busyIndicatorChanged();
        QTimer::singleShot(5 * 1000, this, SLOT(exitVaultOpenProcessWithFail()));
    }
}


int engineClass::lockNumberEntry(int pinCodeNumber)
{
    /* 99 is 'enter' */
    if ( pinCodeNumber == 99 ) {


        if ( m_vaultModeActive == true ) {
            /* VAULT mode */
            QString vaultPinCode = m_lockScreenPinCode;
            if ( vaultPinCode.length() > 3 ) {
                vaultOpenProcess.connect(&vaultOpenProcess,
                                         &QProcess::readyReadStandardOutput,
                                         this, &engineClass::onVaultProcessReadyReadStdOutput);
                vaultOpenProcess.connect(&vaultOpenProcess,
                                         (void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished,
                                         this, &engineClass::onVaultProcessFinished);

                m_vaultNotifyText = "CHECKING";
                m_vaultNotifyColor = "yellow";
                m_vaultNotifyTextColor = "red";
                m_busyIndicatorActive = true;
                emit vaultScreenNotifyTextChanged();
                emit vaultScreenNotifyColorChanged();
                emit vaultScreenNotifyTextColorChanged();
                emit busyIndicatorChanged();
                m_lockScreenPinCode="••••";
                emit lockScreenPinCodeChanged();
                vaultOpenProcess.setProgram("/bin/vault-pin.sh");   // todo:
                vaultOpenProcess.setArguments({vaultPinCode, QString::number(m_vaultIndex) });
                vaultOpenProcess.start();
            } else {
                m_lockScreenPinCode="";
                emit lockScreenPinCodeChanged();
            }
        } else {
            /* NORMAL mode */
            if ( m_lockScreenPinCode.compare( uPref.m_pinCode ) == 0 ) {
                m_lockScreenPinCode="";
                emit lockScreenPinCodeChanged();
                m_lockScreen_active=false;
                emit lockScreen_activeChanged();
                return 0;
            }
            if ( m_lockScreenPinCode.compare( "1234" ) == 0 ) {
                m_lockScreenPinCode="";
                emit lockScreenPinCodeChanged();
                m_lockScreen_active=false;
                emit lockScreen_activeChanged();
                m_camoScreen_active = true;
                emit camoScreen_activeChanged();
                return 0;
            }
        }
        m_lockScreenPinCode="";
        emit lockScreenPinCodeChanged();
    }

    /* backspace */
    if ( pinCodeNumber == 100 && m_lockScreenPinCode.length() > 0 ) {
        m_lockScreenPinCode.chop(1);
        emit lockScreenPinCodeChanged();
        return 0;
    }

    /* show number */
    if ( m_lockScreenPinCode.length() < 6 && pinCodeNumber != 100 && pinCodeNumber != 99 ) {
        m_lockScreenPinCode = m_lockScreenPinCode + QString::number(pinCodeNumber);
        emit lockScreenPinCodeChanged();
    }
    return 0;
}

QString engineClass::getLockScreenPinCode()
{
    return m_lockScreenPinCode;
}

bool engineClass::getGoSecureButton_active()
{
    return m_goSecureButton_active;
}

/* TODO: What is return value is NULL to QML ? <= wtf is this ? */
QString engineClass::getStatusMessage()
{
    return m_statusMessage;
}

QString engineClass::getMyCallSign()
{
    if ( nodes.myNodeName == NULL ) {
        return "wait";
    } else {
        return nodes.myNodeName;
    }
}

QString engineClass::getCallSignInsigniaImage()
{
    return m_callSignInsigniaImage;
}
QString engineClass::getInsigniaLabelText()
{
    return m_insigniaLabelText;
    QString m_insigniaLabelStateText="";
}
QString engineClass::getInsigniaLabelStateText()
{
    return m_insigniaLabelStateText;
}

int engineClass::getSwipeViewIndex()
{
    return m_SwipeViewIndex;
}

bool engineClass::getCallDialogVisible()
{
    return m_callDialogVisible;
}

QString engineClass::getVoltageValue()
{
    return mVoltage;
}

QString engineClass::getVoltageNotifyColor()
{
    return mvoltageNotifyColor;
}

QString engineClass::getNetworkStatusLabel()
{
    return mnetworkStatusLabelValue;
}
QString engineClass::getNetworkStatusLabelColor()
{
    return mnetworkStatusLabelColor;
}

void engineClass::activateInsignia(int node_id, QString stateText)
{
    // Set insignia image (0=alpha etc)
    if (node_id==0) {
        m_callSignInsigniaImage="alpha.png";
        m_insigniaLabelText=nodes.node_name[node_id];
        m_insigniaLabelStateText=stateText;
        emit callSignInsigniaImageChanged();
        emit insigniaLabelTextChanged();
        emit insigniaLabelStateTextChanged();
    }
    if (node_id==1) {
        m_callSignInsigniaImage="bravo.png";
        m_insigniaLabelText=nodes.node_name[node_id];
        m_insigniaLabelStateText=stateText;
        emit callSignInsigniaImageChanged();
        emit insigniaLabelTextChanged();
        emit insigniaLabelStateTextChanged();
    }
    if (node_id==2) {
        m_callSignInsigniaImage="charlie.png";
        m_insigniaLabelText=nodes.node_name[node_id];
        m_insigniaLabelStateText=stateText;
        emit callSignInsigniaImageChanged();
        emit insigniaLabelTextChanged();
        emit insigniaLabelStateTextChanged();
    }
    if (node_id==3) {
        m_callSignInsigniaImage="delta.png";
        m_insigniaLabelText=nodes.node_name[node_id];
        m_insigniaLabelStateText=stateText;
        emit callSignInsigniaImageChanged();
        emit insigniaLabelTextChanged();
        emit insigniaLabelStateTextChanged();
    }
    if (node_id==4) {
        m_callSignInsigniaImage="echo.png";
        m_insigniaLabelText=nodes.node_name[node_id];
        m_insigniaLabelStateText=stateText;
        emit callSignInsigniaImageChanged();
        emit insigniaLabelTextChanged();
        emit insigniaLabelStateTextChanged();
    }
    if (node_id==5) {
        m_callSignInsigniaImage="foxrot.png";
        m_insigniaLabelText=nodes.node_name[node_id];
        m_insigniaLabelStateText=stateText;
        emit callSignInsigniaImageChanged();
        emit insigniaLabelTextChanged();
        emit insigniaLabelStateTextChanged();
    }
    if (node_id==6) {
        m_callSignInsigniaImage="golf.png";
        m_insigniaLabelText=nodes.node_name[node_id];
        m_insigniaLabelStateText=stateText;
        emit callSignInsigniaImageChanged();
        emit insigniaLabelTextChanged();
        emit insigniaLabelStateTextChanged();
    }
    if (node_id==7) {
        m_callSignInsigniaImage="hotel.png";
        m_insigniaLabelText=nodes.node_name[node_id];
        m_insigniaLabelStateText=stateText;
        emit callSignInsigniaImageChanged();
        emit insigniaLabelTextChanged();
        emit insigniaLabelStateTextChanged();
    }
    if (node_id==8) {
        m_callSignInsigniaImage="india.png";
        m_insigniaLabelText=nodes.node_name[node_id];
        m_insigniaLabelStateText=stateText;
        emit callSignInsigniaImageChanged();
        emit insigniaLabelTextChanged();
        emit insigniaLabelStateTextChanged();
    }
    if (node_id==9) {
        m_callSignInsigniaImage="juliet.png";
        m_insigniaLabelText=nodes.node_name[node_id];
        m_insigniaLabelStateText=stateText;
        emit callSignInsigniaImageChanged();
        emit insigniaLabelTextChanged();
        emit insigniaLabelStateTextChanged();
    }


}

/* Connect to peer buttons pressed with ID */
void engineClass::connectButton(int node_id)
{
    eraseConnectionLabels();
    QString scanCmd = nodes.node_ip[node_id] + ",status";
    fifoWrite(scanCmd);
    activateInsignia(node_id, "Selected");
    // TODO: Can we say 'connected to' - without verified connect status?
    QString connectStatusString = "Connected to " + nodes.node_name[node_id];
    updateCallStatusIndicator(connectStatusString, "green", "transparent",LOG_AND_INDICATE);
}

/* Terminate button */
void engineClass::disconnectButton()
{
    if ( g_connectState ) {
        disconnectAsClient(g_connectedNodeIp, g_connectedNodeId);
    }
    updateCallStatusIndicator("Connection terminated.", "green", "transparent",LOG_AND_INDICATE);
    m_callSignInsigniaImage = "";
    m_insigniaLabelText = "";
    m_insigniaLabelStateText = "";
    emit callSignInsigniaImageChanged();
    emit insigniaLabelTextChanged();
    emit insigniaLabelStateTextChanged();
    m_goSecureButton_active = false;
    emit goSecureButton_activeChanged();
    m_SwipeViewIndex = 0;
    emit swipeViewIndexChanged();
    if ( m_messageEraseEnabled ) {
        m_textMsgDisplay = "";
        emit textMsgDisplayChanged();
    }
    m_callDialogVisible = false;
    emit callDialogVisibleChanged();
    eraseConnectionLabels();
    reloadKeyUsage();
    mAudioDeviceBusy = false;
}

/* Popup buttons for CALL dialog */
void engineClass::on_answerButton_clicked()
{
    updateCallStatusIndicator("Accepted", "green","transparent",LOG_AND_INDICATE);

    // Send UI indication that we answered succesfully (TEST) WORK IN PROGRESS
    QString answerString = g_connectedNodeIp + ",message,answer_success";
    fifoWrite( answerString );
    // Wait FIFO with timeout
    if ( waitForFifoReply() == FIFO_TIMEOUT ) {
        updateCallStatusIndicator("Timeout. Aborting.", "green", "transparent",LOG_ONLY );
        return;
    }

    // Send answer to telemetry server
    answerString = g_connectedNodeIp + ",answer";
    fifoWrite( answerString );
    // Wait FIFO with timeout
    if ( waitForFifoReply() == FIFO_TIMEOUT ) {
        updateCallStatusIndicator("Timeout. Aborting.", "green", "transparent",LOG_ONLY );
        return;
    }

    // Connect audio as Server
    answerString = "127.0.0.1,connect_audio_as_server";
    fifoWrite( answerString );
    updateCallStatusIndicator("Audio connected", "green","transparent",INDICATE_ONLY);
    mAudioDeviceBusy = true;
}

void engineClass::on_denyButton_clicked()
{
    QString hangupCommandString = g_connectedNodeIp + ",hangup";
    fifoWrite( hangupCommandString );

    if ( waitForFifoReply() == FIFO_TIMEOUT ) {
        updateCallStatusIndicator("Timeout. Aborting.", "green", "transparent",LOG_ONLY );
        return;
    }

    // Turn off local audio
    hangupCommandString = "127.0.0.1,disconnect_audio";
    fifoWrite( hangupCommandString );
    updateCallStatusIndicator("Incoming Terminated", "green","transparent",LOG_AND_INDICATE);

    // Erase gree status, insignia
    m_callDialogVisible = false;
    emit callDialogVisibleChanged();
    eraseConnectionLabels();
    m_callSignInsigniaImage = "";
    m_insigniaLabelText = "";
    m_insigniaLabelStateText = "";
    emit callSignInsigniaImageChanged();
    emit insigniaLabelTextChanged();
    emit insigniaLabelStateTextChanged();
}

void engineClass::on_LineEdit_returnPressed(QString message)
{
    if ( g_connectState ) {
        m_textMsgDisplay = m_textMsgDisplay + "<br> <font color='" + mMessageColorLocal + "'>" + message + "</font>";
        emit textMsgDisplayChanged();
        message.replace( ",", QChar(SUBSTITUTE_CHAR_CODE) );
        QString fifo_command = g_remoteOtpPeerIp + ",message," + message;
        fifoWrite(fifo_command);
    } else {
        m_textMsgDisplay = "<font color='#00ff00'>NOTE: You are not connected!</font>";
        emit textMsgDisplayChanged();
    }
}

/* Timeout for FIFO replies */
int engineClass::waitForFifoReply() {
    g_fifoCheckInProgress = true;
    fifoReplyTimer = new QTimer(this);
    fifoReplyTimer->start(10000); // 10 s
    connect(fifoReplyTimer, SIGNAL(timeout()), this, SLOT(checkFifoReplyTimeout()) );

        while ( g_fifoReply == "" && g_fifoCheckInProgress == true ) {
            QCoreApplication::processEvents();
            if ( g_fifoReply != "" ) {
                fifoReplyTimer->stop();
                return FIFO_REPLY_RECEIVED;
            }
        }

    if ( g_fifoReply == "" && g_fifoCheckInProgress == false )
        return FIFO_TIMEOUT;
}

int engineClass::checkFifoReplyTimeout() {
    g_fifoCheckInProgress = false;
    fifoReplyTimer->stop();
    return 0;
}


QString engineClass::getTextMsgDisplay()
{
    return m_textMsgDisplay;
}

long int engineClass::get_file_size(QString keyFilename)
{
    long int size = 0;
    QFile myFile(keyFilename);
    if (myFile.open(QIODevice::ReadOnly)){
        size = myFile.size();
        myFile.close();
        return size;
    }
    return -1;
}

long int engineClass::get_key_index(QString counterFilename)
{
    long int index=0;
    FILE *keyindex_file;
    std::string str = counterFilename.toStdString();
    const char* filename = str.c_str();
    keyindex_file = fopen(filename, "rb");
    fread(&index, sizeof(long int),1,keyindex_file);
    fclose(keyindex_file);
    return index;
}

/* WIFI */

QString engineClass::getWifiStatusText()
{
    return m_wifiStatusText;
}

QString engineClass::getWifiNotifyText()
{
    return m_wifiNotifyText;
}
QString engineClass::getWifiNotifyColor()
{
    return m_wifiNotifyColor;
}

void engineClass::wifiScanButton()
{
    m_wifiStatusText = "Scanning...";
    emit wifiStatusTextChanged();
    scanAvailableWifiNetworks("/opt/tunnel/wifi_getnetworks.sh",{""});
}

void engineClass::wifiConnectButton(QString ssid, QString psk)
{
    QStringList parameters={"--passphrase",psk,"station","wlan0","connect",ssid};
    connectWifiNetwork("iwctl", parameters);
    QTimer::singleShot(10000, this, SLOT( getWifiStatus() ));
    m_wifiStatusText = "Connecting, wait 10 seconds. ";
    emit wifiStatusTextChanged();
}

void engineClass::wifiEraseAllConnection()
{
    QProcess process;
    process.setProgram("/bin/nukewifi.sh");
    process.setArguments({""});
    process.start();
    process.waitForFinished();
    m_wifiStatusText = "Connections removed. Power off unit!";
    emit wifiStatusTextChanged();
    m_wifiNotifyColor = "#FF0000";
    emit wifiNotifyColorChanged();

}

QStringList engineClass::getWifiNetworks()
{
    return m_wifiNetworks;
}

QStringList engineClass::getVaultNames()
{
    return m_vaultNames;
}

void engineClass::scanAvailableWifiNetworks(QString command, QStringList parameters)
{
    QProcess process;
    process.setProgram(command);
    process.setArguments(parameters);
    process.start();
    process.waitForFinished();
    QString result=process.readAllStandardOutput();
    QString trimmedList = result.trimmed();

    /*  TODO: Improve this after modified /opt/tunnel/wifi_getnetworks.sh
        The SSID can be any alphanumeric, case-sensitive entry from 2 to 32 characters.
        The printable characters plus the space (ASCII 0x20) are allowed,
        but these six characters are not: ?, ", $, [, \, ], and +.
    */
    QStringList networks=trimmedList.split(" ");
    m_wifiNetworks=trimmedList.split(" ");
    emit wifiNetworksChanged();
    m_wifiStatusText = "Scan ready, found " + QString::number( networks.length() ) + " networks.";
    emit wifiStatusTextChanged();
}
void engineClass::connectWifiNetwork(QString command, QStringList parameters)
{
    qint64 pid;
    QProcess process;
    process.setProgram(command);
    process.setArguments(parameters);
    process.startDetached(&pid);
}
void engineClass::getWifiStatus()
{
    QProcess process;
    process.setProgram("/opt/tunnel/wifi_status.sh");
    process.setArguments({""});
    process.start();
    process.waitForFinished();
    QString result=process.readAllStandardOutput();
    m_wifiStatusText = result;
    emit wifiStatusTextChanged();

    if ( result.contains( "connected",Qt::CaseInsensitive ) ) {
        m_wifiNotifyColor = mMainColor;
        emit wifiNotifyColorChanged();
    }
}

void engineClass::registerTouch()
{
    m_screenTimeoutCounter = DEVICE_LOCK_TIME;
    if ( !g_connectState && m_automaticShutdownEnabled ) {
        automaticShutdownTimer->start( AUTOMATIC_SHUTDOWNTIME );
    }
}

void engineClass::getKnownWifiNetworks()
{
    QProcess process;
    process.setProgram("/opt/tunnel/wifi_getknownnetworks.sh");
    process.setArguments({""});
    process.start();
    process.waitForFinished();
    QString result=process.readAllStandardOutput();
    QString trimmedList = result.trimmed();
    QStringList networks=trimmedList.split(" ");
    // m_wifiNetworks=trimmedList.split(" ");
    // emit wifiNetworksChanged();
}

QString engineClass::getAboutTextContent()
{
    return m_aboutText;
}

void engineClass::loadAboutText()
{
    QString line;
    QFile file("/root/license.txt");
    if(!file.open(QIODevice::ReadOnly)) {
        m_aboutText = "license.txt missing";
    }
    QTextStream in(&file);
    while(!in.atEnd()) {
         line = in.readLine();
         m_aboutText = m_aboutText + line + "\n";
    }
    file.close();
    emit aboutTextContentChanged();
}

bool engineClass::deepSleepEnabled() const
{
    return m_deepSleepEnabled;
}

void engineClass::setDeepSleepEnabled(bool newDeepSleepEnabled)
{
    if (m_deepSleepEnabled == newDeepSleepEnabled)
        return;
    m_deepSleepEnabled = newDeepSleepEnabled;
    emit deepSleepEnabledChanged();
    QSettings settings(USER_PREF_INI_FILE,QSettings::IniFormat);
    settings.setValue("deepsleep", m_deepSleepEnabled);
}

void engineClass::changeDeepSleepEnabled(bool newDeepSleepEnabled)
{
    setDeepSleepEnabled(newDeepSleepEnabled);
}

/* LTE - mobile data
    NOTE: ini-file persist only checkbox state, actual connectivity
          is controlled by systemd service (lte.service) on boot.
*/
bool engineClass::lteEnabled() const
{
    return m_lteEnabled;
}

void engineClass::setLteEnabled(bool newLteEnabled)
{
    if (m_lteEnabled == newLteEnabled)
        return;
    m_lteEnabled = newLteEnabled;
    emit lteEnabledChanged();
    QSettings settings(USER_PREF_INI_FILE,QSettings::IniFormat);
    settings.setValue("lte", m_lteEnabled);
}

void engineClass::changeLteEnabled(bool newLteEnabled)
{
    if ( newLteEnabled )
        runExternalCmd("/bin/systemctl", {"enable", "lte"});
    else
        runExternalCmd("/bin/systemctl", {"disable", "lte"});

    setLteEnabled(newLteEnabled);
}

/* Cell display */
bool engineClass::lteCellDisplayEnabled() const
{
    return m_lteCellDisplayEnabled;
}

void engineClass::setLteCellDisplayEnabled(bool newLteCellDisplayEnabled)
{
    if (m_lteCellDisplayEnabled == newLteCellDisplayEnabled)
        return;
    m_lteCellDisplayEnabled = newLteCellDisplayEnabled;
    emit lteCellDisplayEnabledChanged();
    QSettings settings(USER_PREF_INI_FILE,QSettings::IniFormat);
    settings.setValue("celldisplay", m_lteCellDisplayEnabled);
}

/* Save APN to env file */
void engineClass::apnSaveButton(QString apn)
{
    mApnName = apn;
    QString filename="/root/utils/apn.env";
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);
             out << "APN=" << apn;
    }
    file.close();
}

QString engineClass::getApnName()
{
    return mApnName;
}

bool engineClass::getMacsecPttEnabled()
{
    return mMacsecPttEnabled;
}

void engineClass::setMacsecPttEnabled(bool newPttValue)
{
    if ( mMacsecPttEnabled == newPttValue )
        return;
    mMacsecPttEnabled = newPttValue;
    emit macsecPttEnabledChanged();
    QSettings settings(USER_PREF_INI_FILE,QSettings::IniFormat);
    settings.setValue("ptt", mMacsecPttEnabled);
}

QString engineClass::getMacsecKeyed()
{
    return mMacsecKeyed;
}

bool engineClass::getLayer2Wifi()
{
    return mLayer2WifiEnabled;
}

void engineClass::setLayer2Wifi(bool newLayer2Value)
{
    if ( mLayer2WifiEnabled == newLayer2Value)
        return;
    mLayer2WifiEnabled = newLayer2Value;
    emit layer2WifiChanged();
    QSettings settings(USER_PREF_INI_FILE,QSettings::IniFormat);
    settings.setValue("layer2wifi", mLayer2WifiEnabled);
    QSettings iwd_settings(IWD_MAIN_CONFIG_FILE,QSettings::IniFormat);
    iwd_settings.setValue("EnableNetworkConfiguration", !mLayer2WifiEnabled);
}

bool engineClass::getMacsecValid()
{
    return mMacsecKeyValid;
}

bool engineClass::getBusyIndicator()
{
    return m_busyIndicatorActive;
}

// Get vault index from vault pin page
void engineClass::setVaultId(int vaultIndex)
{
    automaticShutdownTimer->start( AUTOMATIC_SHUTDOWNTIME_IN_VAULT_MODE );
    m_vaultIndex = vaultIndex;
}

// Load APN from file and default to 'internet' if no file is present
void engineClass::loadApnName()
{
    QString apnFromFile;
    QString filename="/root/utils/apn.env";
    QFile file(filename);
    if(!file.exists()){
        apnFromFile = "internet";
    }
    else {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream stream(&file);
            while (!stream.atEnd()){
                apnFromFile = stream.readLine();
            }
        }
        file.close();
    }
    QStringList apnElements = apnFromFile.split('=');
    mApnName = apnElements[1];
    emit apnNameChanged();
}

/* Night mode */
bool engineClass::nightModeEnabled() const
{
    return m_nightModeEnabled;
}

void engineClass::setNightModeEnabled(bool newNightModeEnabled)
{
    if (m_nightModeEnabled == newNightModeEnabled)
        return;
    m_nightModeEnabled = newNightModeEnabled;
    emit nightModeEnabledChanged();
    QSettings settings(USER_PREF_INI_FILE,QSettings::IniFormat);
    settings.setValue("nightmode", m_nightModeEnabled);
}

void engineClass::changeNightModeEnabled(bool newNightModeEnabled)
{
    if ( newNightModeEnabled ) {
                                    //  625 nm      green           640 nm
        mMainColor = "#ff2100";     //  #ff6300     #00FF00         ff2100
        emit mainColorChanged();
        mHighColor = "#ff8080";     //  #ffa300     lightgreen      ff6100
        emit highColorChanged();
        mDimColor = "#cc2100";      //  #dd5300                     cc2100
        emit dimColorChanged();
        m_wifiNotifyColor = mMainColor;
        emit wifiNotifyColorChanged();
        mMessageColorLocal = mDimColor;
        mMessageColorRemote = mMainColor;
    }
    else {
                                    //  625 nm      green           640 nm
        mMainColor = "#00FF00";     //  #ff6300     #00FF00         ff2100
        emit mainColorChanged();
        mHighColor = "lightgreen";  //  #ffa300     lightgreen      ff6100
        emit highColorChanged();
        mDimColor = "#21cc00";      //  #dd5300                     cc2100
        emit dimColorChanged();
        m_wifiNotifyColor = mMainColor;
        emit wifiNotifyColorChanged();
        mMessageColorLocal = mDimColor;
        mMessageColorRemote = mMainColor;
    }
    setNightModeEnabled(newNightModeEnabled);
}

bool engineClass::callSignOnVaultEnabled() const
{
    return m_callSignVisibleOnVaultPage;
}

void engineClass::setCallSignOnVaultEnabled(bool newCallSignOnVaultEnabled)
{
    if (m_callSignVisibleOnVaultPage == newCallSignOnVaultEnabled)
        return;
    m_callSignVisibleOnVaultPage = newCallSignOnVaultEnabled;
    emit callSignOnVaultEnabledChanged();
    QSettings settings(PRE_VAULT_INI_FILE,QSettings::IniFormat);
    settings.setValue("vaultpagecallsign", m_callSignVisibleOnVaultPage);
    settings.setValue("my_name", nodes.myNodeName);
}

bool engineClass::messageEraseEnabled() const
{
    return m_messageEraseEnabled;
}

void engineClass::setMessageEraseEnabled(bool newMessageEraseEnabled)
{
    if (m_messageEraseEnabled == newMessageEraseEnabled)
        return;
    m_messageEraseEnabled = newMessageEraseEnabled;
    emit messageEraseEnabledChanged();
    QSettings settings(PRE_VAULT_INI_FILE,QSettings::IniFormat);
    settings.setValue("msg_erase", m_messageEraseEnabled);
}

// automaticShutdownEnabled
bool engineClass::automaticShutdownEnabled() const
{
    return m_automaticShutdownEnabled;
}

void engineClass::setAutomaticShutdownEnabled(bool newAutomaticShutdownEnabled)
{
    if (m_automaticShutdownEnabled == newAutomaticShutdownEnabled)
        return;
    m_automaticShutdownEnabled = newAutomaticShutdownEnabled;
    emit automaticShutdownEnabledChanged();
    QSettings settings(PRE_VAULT_INI_FILE,QSettings::IniFormat);
    settings.setValue("automaticshutdown", m_automaticShutdownEnabled);
    if (m_automaticShutdownEnabled) {
        automaticShutdownTimer->start( AUTOMATIC_SHUTDOWNTIME );
    }
    if (!m_automaticShutdownEnabled) {
        automaticShutdownTimer->stop();
    }
}




QString engineClass::getPlmn() {
    return mPlmn;
}
QString engineClass::getTa() {
    return mTa;
}
QString engineClass::getGc() {
    return mGc;
}

QString engineClass::getSc() {
    return mSc;
}
QString engineClass::getAc() {
    return mAc;
}
QString engineClass::getRssi() {
    return mRssi;
}
QString engineClass::getRsrq() {
    return mRsrq;
}
QString engineClass::getRsrp() {
    return mRsrp;
}
QString engineClass::getSnr() {
    return mSnr;
}




