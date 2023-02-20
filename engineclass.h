/*  Small Pine phone QML interface for Out-Of-Band communication.

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

*/
#ifndef ENGINECLASS_H
#define ENGINECLASS_H
#include <QObject>
#include <QFile>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QSocketNotifier>
#include <QProcess>
#include <QQmlPropertyMap>

#define PEER_COUNT  10
#define NODECOUNT   10
#define CONNPOINTCOUNT 3
#define TELEMETRY_FIFO_IN       "/tmp/telemetry_fifo_in"
#define TELEMETRY_FIFO_OUT      "/tmp/telemetry_fifo_out"
#define MESSAGE_RECEIVE_FIFO    "/tmp/message_fifo_out"
#define DEVICE_LOCK_TIME        60


class engineClass : public QObject
{
    Q_OBJECT



    //                               c++ returns private variable:
    //                               return m_peerCallSign[0];
    //                               |
    //                 QML:          |                    c++ emit this signal if m_peerCallSign is updated
    //                 text: eClass.peerName              |
    //                 |             |                    |
    Q_PROPERTY(QString peer_0_Name READ getPeer_0_Name() NOTIFY peer_0_NameChanged);
    Q_PROPERTY(QString peer_1_Name READ getPeer_1_Name() NOTIFY peer_1_NameChanged);
    Q_PROPERTY(QString peer_2_Name READ getPeer_2_Name() NOTIFY peer_2_NameChanged);
    Q_PROPERTY(QString peer_3_Name READ getPeer_3_Name() NOTIFY peer_3_NameChanged);
    Q_PROPERTY(QString peer_4_Name READ getPeer_4_Name() NOTIFY peer_4_NameChanged);
    Q_PROPERTY(QString peer_5_Name READ getPeer_5_Name() NOTIFY peer_5_NameChanged);
    Q_PROPERTY(QString peer_6_Name READ getPeer_6_Name() NOTIFY peer_6_NameChanged);
    Q_PROPERTY(QString peer_7_Name READ getPeer_7_Name() NOTIFY peer_7_NameChanged);
    Q_PROPERTY(QString peer_8_Name READ getPeer_8_Name() NOTIFY peer_8_NameChanged);
    Q_PROPERTY(QString peer_9_Name READ getPeer_9_Name() NOTIFY peer_9_NameChanged);

    Q_PROPERTY(QString peer_0_NameColor READ getPeer_0_NameColor() NOTIFY peer_0_NameColorChanged);
    Q_PROPERTY(QString peer_1_NameColor READ getPeer_1_NameColor() NOTIFY peer_1_NameColorChanged);
    Q_PROPERTY(QString peer_2_NameColor READ getPeer_2_NameColor() NOTIFY peer_2_NameColorChanged);
    Q_PROPERTY(QString peer_3_NameColor READ getPeer_3_NameColor() NOTIFY peer_3_NameColorChanged);
    Q_PROPERTY(QString peer_4_NameColor READ getPeer_4_NameColor() NOTIFY peer_4_NameColorChanged);
    Q_PROPERTY(QString peer_5_NameColor READ getPeer_5_NameColor() NOTIFY peer_5_NameColorChanged);
    Q_PROPERTY(QString peer_6_NameColor READ getPeer_6_NameColor() NOTIFY peer_6_NameColorChanged);
    Q_PROPERTY(QString peer_7_NameColor READ getPeer_7_NameColor() NOTIFY peer_7_NameColorChanged);
    Q_PROPERTY(QString peer_8_NameColor READ getPeer_8_NameColor() NOTIFY peer_8_NameColorChanged);
    Q_PROPERTY(QString peer_9_NameColor READ getPeer_9_NameColor() NOTIFY peer_9_NameColorChanged);

    Q_PROPERTY(bool peer_0_label READ getPeer_0_Label() NOTIFY peer_0_LabelChanged);
    Q_PROPERTY(bool peer_1_label READ getPeer_1_Label() NOTIFY peer_1_LabelChanged);
    Q_PROPERTY(bool peer_2_label READ getPeer_2_Label() NOTIFY peer_2_LabelChanged);
    Q_PROPERTY(bool peer_3_label READ getPeer_3_Label() NOTIFY peer_3_LabelChanged);
    Q_PROPERTY(bool peer_4_label READ getPeer_4_Label() NOTIFY peer_4_LabelChanged);
    Q_PROPERTY(bool peer_5_label READ getPeer_5_Label() NOTIFY peer_5_LabelChanged);
    Q_PROPERTY(bool peer_6_label READ getPeer_6_Label() NOTIFY peer_6_LabelChanged);
    Q_PROPERTY(bool peer_7_label READ getPeer_7_Label() NOTIFY peer_7_LabelChanged);
    Q_PROPERTY(bool peer_8_label READ getPeer_8_Label() NOTIFY peer_8_LabelChanged);
    Q_PROPERTY(bool peer_9_label READ getPeer_9_Label() NOTIFY peer_9_LabelChanged);

    Q_PROPERTY(QString peer_0_label_color READ getPeer_0_Label_color() NOTIFY peer_0_LabelColorChanged);
    Q_PROPERTY(QString peer_1_label_color READ getPeer_1_Label_color() NOTIFY peer_1_LabelColorChanged);
    Q_PROPERTY(QString peer_2_label_color READ getPeer_2_Label_color() NOTIFY peer_2_LabelColorChanged);
    Q_PROPERTY(QString peer_3_label_color READ getPeer_3_Label_color() NOTIFY peer_3_LabelColorChanged);
    Q_PROPERTY(QString peer_4_label_color READ getPeer_4_Label_color() NOTIFY peer_4_LabelColorChanged);
    Q_PROPERTY(QString peer_5_label_color READ getPeer_5_Label_color() NOTIFY peer_5_LabelColorChanged);
    Q_PROPERTY(QString peer_6_label_color READ getPeer_6_Label_color() NOTIFY peer_6_LabelColorChanged);
    Q_PROPERTY(QString peer_7_label_color READ getPeer_7_Label_color() NOTIFY peer_7_LabelColorChanged);
    Q_PROPERTY(QString peer_8_label_color READ getPeer_8_Label_color() NOTIFY peer_8_LabelColorChanged);
    Q_PROPERTY(QString peer_9_label_color READ getPeer_9_Label_color() NOTIFY peer_9_LabelColorChanged);

    Q_PROPERTY(bool button_0_active READ getButton_0_active() NOTIFY button_0_activeChanged);
    Q_PROPERTY(bool button_1_active READ getButton_1_active() NOTIFY button_1_activeChanged);
    Q_PROPERTY(bool button_2_active READ getButton_2_active() NOTIFY button_2_activeChanged);
    Q_PROPERTY(bool button_3_active READ getButton_3_active() NOTIFY button_3_activeChanged);
    Q_PROPERTY(bool button_4_active READ getButton_4_active() NOTIFY button_4_activeChanged);
    Q_PROPERTY(bool button_5_active READ getButton_5_active() NOTIFY button_5_activeChanged);
    Q_PROPERTY(bool button_6_active READ getButton_6_active() NOTIFY button_6_activeChanged);
    Q_PROPERTY(bool button_7_active READ getButton_7_active() NOTIFY button_7_activeChanged);
    Q_PROPERTY(bool button_8_active READ getButton_8_active() NOTIFY button_8_activeChanged);
    Q_PROPERTY(bool button_9_active READ getButton_9_active() NOTIFY button_9_activeChanged);

    Q_PROPERTY(bool goSecureButton_active READ getGoSecureButton_active() NOTIFY goSecureButton_activeChanged);
    Q_PROPERTY(bool callDialogVisible READ getCallDialogVisible() NOTIFY callDialogVisibleChanged);

    Q_PROPERTY(QString statusMessage READ getStatusMessage() NOTIFY statusMessageChanged);
    Q_PROPERTY(QString myCallSign READ getMyCallSign() NOTIFY myCallSignChanged);
    Q_PROPERTY(QString callSignInsigniaImage READ getCallSignInsigniaImage() NOTIFY callSignInsigniaImageChanged);
    Q_PROPERTY(QString insigniaLabelText READ getInsigniaLabelText() NOTIFY insigniaLabelTextChanged);
    Q_PROPERTY(QString insigniaLabelStateText READ getInsigniaLabelStateText() NOTIFY insigniaLabelStateTextChanged);
    Q_PROPERTY(QString textMsgDisplay READ getTextMsgDisplay() NOTIFY textMsgDisplayChanged);
    Q_PROPERTY(int swipeViewIndex READ getSwipeViewIndex() NOTIFY swipeViewIndexChanged);

    Q_PROPERTY(QString voltageValue READ getVoltageValue() NOTIFY voltageValueChanged);
    Q_PROPERTY(QString voltageNotifyColor READ getVoltageNotifyColor() NOTIFY voltageNotifyColorChanged);
    Q_PROPERTY(QString networkStatusLabelValue READ getNetworkStatusLabel() NOTIFY networkStatusLabelChanged);
    Q_PROPERTY(QString networkStatusLabelColor READ getNetworkStatusLabelColor() NOTIFY networkStatusLabelColorChanged);
    Q_PROPERTY(bool touchBlock_active READ getTouchBlock_active() NOTIFY touchBlock_activeChanged);
    Q_PROPERTY(bool lockScreen_active READ getLockScreen_active() NOTIFY lockScreen_activeChanged);
    Q_PROPERTY(bool camoScreen_active READ getCamoScreen_active() NOTIFY camoScreen_activeChanged);
    Q_PROPERTY(bool vaultScreen_active READ getVaultScreen_active() NOTIFY vaultScreen_activeChanged);
    Q_PROPERTY(QString vaultScreenNotifyText READ getVaultScreenNotifyText() NOTIFY vaultScreenNotifyTextChanged);
    Q_PROPERTY(QString vaultScreenNotifyColor READ getVaultScreenNotifyColor() NOTIFY vaultScreenNotifyColorChanged);
    Q_PROPERTY(QString vaultScreenNotifyTextColor READ getVaultScreenNotifyTextColor() NOTIFY vaultScreenNotifyTextColorChanged);

    // Key percentage
    Q_PROPERTY(QString peer_0_keyPercentage READ getPeer_0_keyPercentage() NOTIFY peer_0_keyPercentageChanged);
    Q_PROPERTY(QString peer_1_keyPercentage READ getPeer_1_keyPercentage() NOTIFY peer_1_keyPercentageChanged);
    Q_PROPERTY(QString peer_2_keyPercentage READ getPeer_2_keyPercentage() NOTIFY peer_2_keyPercentageChanged);
    Q_PROPERTY(QString peer_3_keyPercentage READ getPeer_3_keyPercentage() NOTIFY peer_3_keyPercentageChanged);
    Q_PROPERTY(QString peer_4_keyPercentage READ getPeer_4_keyPercentage() NOTIFY peer_4_keyPercentageChanged);
    Q_PROPERTY(QString peer_5_keyPercentage READ getPeer_5_keyPercentage() NOTIFY peer_5_keyPercentageChanged);
    Q_PROPERTY(QString peer_6_keyPercentage READ getPeer_6_keyPercentage() NOTIFY peer_6_keyPercentageChanged);
    Q_PROPERTY(QString peer_7_keyPercentage READ getPeer_7_keyPercentage() NOTIFY peer_7_keyPercentageChanged);
    Q_PROPERTY(QString peer_8_keyPercentage READ getPeer_8_keyPercentage() NOTIFY peer_8_keyPercentageChanged);
    Q_PROPERTY(QString peer_9_keyPercentage READ getPeer_9_keyPercentage() NOTIFY peer_9_keyPercentageChanged);

    // pin code
    Q_PROPERTY(QString lockScreenPinCode READ getLockScreenPinCode() NOTIFY lockScreenPinCodeChanged);

    // wifi status
    Q_PROPERTY(QString wifiStatusText READ getWifiStatusText() NOTIFY wifiStatusTextChanged);

    // wifi m_wifiNetworks
    Q_PROPERTY(QStringList wifiNetworks READ getWifiNetworks() NOTIFY wifiNetworksChanged);
    // Wifi notify on top bar
    Q_PROPERTY(QString wifiNotifyText READ getWifiNotifyText() NOTIFY wifiNotifyTextChanged);
    Q_PROPERTY(QString wifiNotifyColor READ getWifiNotifyColor() NOTIFY wifiNotifyColorChanged);
    // About text & deep sleep
    Q_PROPERTY(QString aboutTextContent READ getAboutTextContent() NOTIFY aboutTextContentChanged);
    Q_PROPERTY(bool deepSleepEnabled READ deepSleepEnabled WRITE setDeepSleepEnabled NOTIFY deepSleepEnabledChanged);

    Q_PROPERTY(bool lteEnabled READ lteEnabled WRITE setLteEnabled NOTIFY lteEnabledChanged)
    Q_PROPERTY(bool nightModeEnabled READ nightModeEnabled WRITE setNightModeEnabled NOTIFY nightModeEnabledChanged)

    Q_PROPERTY(bool powerOffDialogVisible READ getPowerOffVisible() NOTIFY powerOffVisibleChanged);

    // Cellular info
    Q_PROPERTY(QString plmn READ getPlmn NOTIFY plmnChanged)
    Q_PROPERTY(QString ta READ getTa NOTIFY taChanged)
    Q_PROPERTY(QString gc READ getGc NOTIFY gcChanged)
    Q_PROPERTY(QString sc READ getSc NOTIFY scChanged)
    Q_PROPERTY(QString ac READ getAc NOTIFY acChanged)
    Q_PROPERTY(QString rssi READ getRssi NOTIFY rssiChanged)
    Q_PROPERTY(QString rsrq READ getRsrq NOTIFY rsrqChanged)
    Q_PROPERTY(QString rsrp READ getRsrp NOTIFY rsrpChanged)
    Q_PROPERTY(QString snr READ getSnr NOTIFY snrChanged)

    // Color
    Q_PROPERTY(QString mainColor READ getMainColor NOTIFY mainColorChanged)
    Q_PROPERTY(QString highColor READ getHighColor NOTIFY highColorChanged)
    Q_PROPERTY(QString dimColor READ getDimColor NOTIFY dimColorChanged)


public:
    explicit engineClass(QObject *parent = nullptr);
    Q_INVOKABLE void debugThis(QString debugMessage);

    // Peers to buttons
    Q_INVOKABLE QString getPeer_0_Name();
    Q_INVOKABLE QString getPeer_1_Name();
    Q_INVOKABLE QString getPeer_2_Name();
    Q_INVOKABLE QString getPeer_3_Name();
    Q_INVOKABLE QString getPeer_4_Name();
    Q_INVOKABLE QString getPeer_5_Name();
    Q_INVOKABLE QString getPeer_6_Name();
    Q_INVOKABLE QString getPeer_7_Name();
    Q_INVOKABLE QString getPeer_8_Name();
    Q_INVOKABLE QString getPeer_9_Name();

    Q_INVOKABLE QString getPeer_0_NameColor();
    Q_INVOKABLE QString getPeer_1_NameColor();
    Q_INVOKABLE QString getPeer_2_NameColor();
    Q_INVOKABLE QString getPeer_3_NameColor();
    Q_INVOKABLE QString getPeer_4_NameColor();
    Q_INVOKABLE QString getPeer_5_NameColor();
    Q_INVOKABLE QString getPeer_6_NameColor();
    Q_INVOKABLE QString getPeer_7_NameColor();
    Q_INVOKABLE QString getPeer_8_NameColor();
    Q_INVOKABLE QString getPeer_9_NameColor();

    Q_INVOKABLE bool getPeer_0_Label();
    Q_INVOKABLE bool getPeer_1_Label();
    Q_INVOKABLE bool getPeer_2_Label();
    Q_INVOKABLE bool getPeer_3_Label();
    Q_INVOKABLE bool getPeer_4_Label();
    Q_INVOKABLE bool getPeer_5_Label();
    Q_INVOKABLE bool getPeer_6_Label();
    Q_INVOKABLE bool getPeer_7_Label();
    Q_INVOKABLE bool getPeer_8_Label();
    Q_INVOKABLE bool getPeer_9_Label();

    Q_INVOKABLE QString getPeer_0_Label_color();
    Q_INVOKABLE QString getPeer_1_Label_color();
    Q_INVOKABLE QString getPeer_2_Label_color();
    Q_INVOKABLE QString getPeer_3_Label_color();
    Q_INVOKABLE QString getPeer_4_Label_color();
    Q_INVOKABLE QString getPeer_5_Label_color();
    Q_INVOKABLE QString getPeer_6_Label_color();
    Q_INVOKABLE QString getPeer_7_Label_color();
    Q_INVOKABLE QString getPeer_8_Label_color();
    Q_INVOKABLE QString getPeer_9_Label_color();

    Q_INVOKABLE bool getButton_0_active();
    Q_INVOKABLE bool getButton_1_active();
    Q_INVOKABLE bool getButton_2_active();
    Q_INVOKABLE bool getButton_3_active();
    Q_INVOKABLE bool getButton_4_active();
    Q_INVOKABLE bool getButton_5_active();
    Q_INVOKABLE bool getButton_6_active();
    Q_INVOKABLE bool getButton_7_active();
    Q_INVOKABLE bool getButton_8_active();
    Q_INVOKABLE bool getButton_9_active();

    Q_INVOKABLE bool getGoSecureButton_active();

    Q_INVOKABLE QString getStatusMessage();
    Q_INVOKABLE QString getMyCallSign();

    /* Connect to peer, disconnect and call dialog */
    Q_INVOKABLE void connectButton(int node_id);
    Q_INVOKABLE void disconnectButton();
    Q_INVOKABLE void on_goSecure_clicked();
    Q_INVOKABLE void on_answerButton_clicked();
    Q_INVOKABLE void on_denyButton_clicked();
    Q_INVOKABLE bool getCallDialogVisible();

    /* Touch prevention frame */
    Q_INVOKABLE bool getTouchBlock_active();
    Q_INVOKABLE bool getLockScreen_active();
    Q_INVOKABLE int lockNumberEntry(int pinCodeNumber );
    Q_INVOKABLE bool getCamoScreen_active();
    Q_INVOKABLE bool getVaultScreen_active();

    Q_INVOKABLE QString getVaultScreenNotifyText();
    Q_INVOKABLE QString getVaultScreenNotifyColor();
    Q_INVOKABLE QString getVaultScreenNotifyTextColor();

    /* Insignia */
    Q_INVOKABLE QString getCallSignInsigniaImage();
    Q_INVOKABLE QString getInsigniaLabelText();
    Q_INVOKABLE QString getInsigniaLabelStateText();

    /* Messaging */
    Q_INVOKABLE void on_LineEdit_returnPressed(QString message);
    Q_INVOKABLE QString getTextMsgDisplay();

    /* UI */
    Q_INVOKABLE int getSwipeViewIndex();

    /* Environment */
    Q_INVOKABLE QString getVoltageValue();
    Q_INVOKABLE QString getVoltageNotifyColor();
    Q_INVOKABLE QString getNetworkStatusLabel();
    Q_INVOKABLE QString getNetworkStatusLabelColor();
    Q_INVOKABLE void powerOff();
    Q_INVOKABLE void quickButtonSend(int sendCode);

    Q_INVOKABLE QString getPeer_0_keyPercentage();
    Q_INVOKABLE QString getPeer_1_keyPercentage();
    Q_INVOKABLE QString getPeer_2_keyPercentage();
    Q_INVOKABLE QString getPeer_3_keyPercentage();
    Q_INVOKABLE QString getPeer_4_keyPercentage();
    Q_INVOKABLE QString getPeer_5_keyPercentage();
    Q_INVOKABLE QString getPeer_6_keyPercentage();
    Q_INVOKABLE QString getPeer_7_keyPercentage();
    Q_INVOKABLE QString getPeer_8_keyPercentage();
    Q_INVOKABLE QString getPeer_9_keyPercentage();
    Q_INVOKABLE QString getLockScreenPinCode();

    Q_INVOKABLE void wifiScanButton();
    Q_INVOKABLE void wifiConnectButton(QString ssid, QString psk);
    Q_INVOKABLE QString getWifiStatusText();
    Q_INVOKABLE QStringList getWifiNetworks();
    Q_INVOKABLE void wifiEraseAllConnection();
    Q_INVOKABLE QString getWifiNotifyText();
    Q_INVOKABLE QString getWifiNotifyColor();

    Q_INVOKABLE QString getAboutTextContent();
    Q_INVOKABLE void getWifiStatus();
    Q_INVOKABLE void registerTouch();

    bool deepSleepEnabled() const;
    void setDeepSleepEnabled(bool newDeepSleepEnabled);
    Q_INVOKABLE void changeDeepSleepEnabled(bool newDeepSleepEnabled);

    bool lteEnabled() const;
    void setLteEnabled(bool newLteEnabled);
    Q_INVOKABLE void changeLteEnabled(bool newLteEnabled);

    bool nightModeEnabled() const;
    void setNightModeEnabled(bool newNightModeEnabled);
    Q_INVOKABLE void changeNightModeEnabled(bool newNightModeEnabled);

    Q_INVOKABLE QString getPlmn();
    Q_INVOKABLE QString getTa();
    Q_INVOKABLE QString getGc();
    Q_INVOKABLE QString getSc();
    Q_INVOKABLE QString getAc();
    Q_INVOKABLE QString getRssi();
    Q_INVOKABLE QString getRsrq();
    Q_INVOKABLE QString getRsrp();
    Q_INVOKABLE QString getSnr();
    Q_INVOKABLE bool getPowerOffVisible();
    Q_INVOKABLE void closePowerOffDialog();
    Q_INVOKABLE void setSwipeIndex(int index);

    Q_INVOKABLE QString appVersion();
    Q_INVOKABLE QString getMainColor();
    Q_INVOKABLE QString getHighColor();
    Q_INVOKABLE QString getDimColor();



/*
    Q_PROPERTY(QString plmn READ getPlmn NOTIFY plmnChanged)
    Q_PROPERTY(QString ta READ  NOTIFY taChanged)
    Q_PROPERTY(QString gc READ  NOTIFY gcChanged)
    Q_PROPERTY(QString sc READ  NOTIFY scChanged)
    Q_PROPERTY(QString ac READ  NOTIFY acChanged)
    Q_PROPERTY(QString rssi READ  NOTIFY rssiChanged)
    Q_PROPERTY(QString rsrq READ  NOTIFY rsrqChanged)
    Q_PROPERTY(QString rsrp READ  NOTIFY rsrpChanged)
    Q_PROPERTY(QString snr READ  NOTIFY snrChanged)
*/

private:
    QString m_peer_0_CallSign="";
    QString m_peer_1_CallSign="";
    QString m_peer_2_CallSign="";
    QString m_peer_3_CallSign="";
    QString m_peer_4_CallSign="";
    QString m_peer_5_CallSign="";
    QString m_peer_6_CallSign="";
    QString m_peer_7_CallSign="";
    QString m_peer_8_CallSign="";
    QString m_peer_9_CallSign="";

    QString m_peer_0_CallSignColor="";
    QString m_peer_1_CallSignColor="";
    QString m_peer_2_CallSignColor="";
    QString m_peer_3_CallSignColor="";
    QString m_peer_4_CallSignColor="";
    QString m_peer_5_CallSignColor="";
    QString m_peer_6_CallSignColor="";
    QString m_peer_7_CallSignColor="";
    QString m_peer_8_CallSignColor="";
    QString m_peer_9_CallSignColor="";

    bool m_peer_0_connection_label = false;
    bool m_peer_1_connection_label = false;
    bool m_peer_2_connection_label = false;
    bool m_peer_3_connection_label = false;
    bool m_peer_4_connection_label = false;
    bool m_peer_5_connection_label = false;
    bool m_peer_6_connection_label = false;
    bool m_peer_7_connection_label = false;
    bool m_peer_8_connection_label = false;
    bool m_peer_9_connection_label = false;

    QString m_peer_0_connection_label_color = "green";
    QString m_peer_1_connection_label_color = "green";
    QString m_peer_2_connection_label_color = "green";
    QString m_peer_3_connection_label_color = "green";
    QString m_peer_4_connection_label_color = "green";
    QString m_peer_5_connection_label_color = "green";
    QString m_peer_6_connection_label_color = "green";
    QString m_peer_7_connection_label_color = "green";
    QString m_peer_8_connection_label_color = "green";
    QString m_peer_9_connection_label_color = "green";

    bool m_button_0_active=false;
    bool m_button_1_active=false;
    bool m_button_2_active=false;
    bool m_button_3_active=false;
    bool m_button_4_active=false;
    bool m_button_5_active=false;
    bool m_button_6_active=false;
    bool m_button_7_active=false;
    bool m_button_8_active=false;
    bool m_button_9_active=false;

    bool m_goSecureButton_active=false;
    bool m_callDialogVisible=false;
    QString m_statusMessage="";
    QString m_callSignInsigniaImage="";
    QString m_insigniaLabelText="";
    QString m_insigniaLabelStateText="";
    QString m_textMsgDisplay="";
    int m_SwipeViewIndex=0;
    QString mVoltage;
    QString mvoltageNotifyColor="#00FF00";
    QString mnetworkStatusLabelValue="";
    QString mnetworkStatusLabelColor="#00FF00";
    QString m_lockScreenPinCode;

    /* System preferences */
    struct SPreferences
    {
        QString node_name[NODECOUNT];
        QString node_ip[NODECOUNT];
        QString node_id[NODECOUNT];
        QString myNodeId;
        QString myNodeIp;
        QString myNodeName;
        QString connectionPointName[CONNPOINTCOUNT];
        QString systemName;
        QString beepActive;
        QString connectionProfile;

    };
    SPreferences nodes;
    bool g_connectState;
    QString g_connectedNodeId;
    QString g_connectedNodeIp;
    QString g_remoteOtpPeerIp;
    QString g_fifoReply;
    double rxKeyRemaining;
    double txKeyRemaining;
    QString txKeyRemainingString;
    QString rxKeyRemainingString;

    /* FIFO */
    QFile fifoIn;
    QFile msgFifoIn;
    QFileSystemWatcher *watcher;
    QTimer *envTimer;

    /* GPIO Notifier */
    QSocketNotifier * m_pwrButtonNotify;
    QSocketNotifier * m_volButtonNotify;
    int m_pwrButtonFileHandle;
    int m_volButtonFileHandle;
    bool m_deviceLocked=false;
    bool m_touchBlock_active=false;
    bool m_lockScreen_active=true;
    bool m_camoScreen_active=false;
    int m_screenTimeoutCounter=DEVICE_LOCK_TIME;
    QString m_keyPersentage_incount[NODECOUNT];
    QString m_keyPersentage_outcount[NODECOUNT];
    QString m_peerLatencyValue[NODECOUNT];
    bool m_vaultModeActive=false;
    QString m_vaultNotifyText;
    QString m_vaultNotifyColor;
    QString m_vaultNotifyTextColor;
    /* Vault open */
    QProcess vaultOpenProcess;
    QTimer *fifoReplyTimer;
    bool g_fifoCheckInProgress;
    int m_SpeakerVolumeRuntimeValue=70;
    QString m_wifiStatusText;

    QStringList m_wifiNetworks;
    QString m_wifiNotifyText;
    QString m_wifiNotifyColor;
    QString m_aboutText;

    /* User preferences */
    struct UserPreferences
    {
        QString volumeValue;
        QString m_pinCode;
        QString m_settingsPinCode;
        QString m_autoerase;
        QString m_micVolume;
        QString m_extraSettingsPinCode;
    };
    UserPreferences uPref;
    void loadUserPreferences();
    void saveUserPreferences();
    bool m_deepSleepEnabled=false;
    bool m_lteEnabled=false;
    bool m_nightModeEnabled=false;

    QString mPlmn;
    QString mTa;
    QString mGc;
    QString mSc;
    QString mAc;
    QString mRssi;
    QString mRsrq;
    QString mRsrp;
    QString mSnr;

    bool mPwrButtonReleased=false;
    bool mPwrButtonCycle=false;
    bool mPowerOffDialog=false;
    QTimer *proximityTimer;
    int mBacklightLevel=50;
    QString mMainColor;
    QString mHighColor;
    QString mDimColor;
    QString mMessageColorLocal;
    QString mMessageColorRemote;



public slots:
    void initEngine();
    void loadSettings();
    void setVaultMode(bool vaultModeActive);

private slots:
    int fifoChanged();
    int msgFifoChanged();
    void fifoWrite(QString message);
    void connectAsClient(QString nodeIp, QString nodeId);
    void disconnectAsClient(QString nodeIp, QString nodeId);
    void updateCallStatusIndicator(QString text, QString fontColor, QString backgroundColor, int logMethod );
    void touchLocalFile(QString filename);
    void removeLocalFile(QString filename);
    void setIndicatorForIncomingConnection(QString peerIp);
    void eraseConnectionLabels();
    void activateInsignia(int node_id, QString stateText);
    void envTimerTick();
    void readPwrGpioButton();
    void readPwrGpioButtonTimer();
    void readVolGpioButton();
    void runExternalCmd(QString command, QStringList parameters );
    void runExternalCmdCaptureOutput(QString command, QStringList parameters);
    void lockDevice(bool state);
    long int get_file_size(QString keyFilename);
    long int get_key_index(QString counterFilename);
    void reloadKeyUsage();
    void onVaultProcessReadyReadStdOutput();
    void onVaultProcessFinished();
    void exitVaultOpenProcess();
    void exitVaultOpenProcessWithFail();
    void peerLatency();
    int waitForFifoReply();
    int checkFifoReplyTimeout();
    void setSystemVolume(int volume);
    void setMicrophoneVolume(int volume);
    void scanAvailableWifiNetworks(QString command, QStringList parameters);
    void connectWifiNetwork(QString command, QStringList parameters);
    void getKnownWifiNetworks();
    void loadAboutText();
    void proximityTimerTick();

signals:
    void peer_0_NameChanged(); // emit this on m_peerCallSign change
    void peer_1_NameChanged();
    void peer_2_NameChanged();
    void peer_3_NameChanged();
    void peer_4_NameChanged();
    void peer_5_NameChanged();
    void peer_6_NameChanged();
    void peer_7_NameChanged();
    void peer_8_NameChanged();
    void peer_9_NameChanged();

    void peer_0_NameColorChanged();
    void peer_1_NameColorChanged();
    void peer_2_NameColorChanged();
    void peer_3_NameColorChanged();
    void peer_4_NameColorChanged();
    void peer_5_NameColorChanged();
    void peer_6_NameColorChanged();
    void peer_7_NameColorChanged();
    void peer_8_NameColorChanged();
    void peer_9_NameColorChanged();

    void peer_0_LabelChanged();
    void peer_1_LabelChanged();
    void peer_2_LabelChanged();
    void peer_3_LabelChanged();
    void peer_4_LabelChanged();
    void peer_5_LabelChanged();
    void peer_6_LabelChanged();
    void peer_7_LabelChanged();
    void peer_8_LabelChanged();
    void peer_9_LabelChanged();

    void peer_0_LabelColorChanged();
    void peer_1_LabelColorChanged();
    void peer_2_LabelColorChanged();
    void peer_3_LabelColorChanged();
    void peer_4_LabelColorChanged();
    void peer_5_LabelColorChanged();
    void peer_6_LabelColorChanged();
    void peer_7_LabelColorChanged();
    void peer_8_LabelColorChanged();
    void peer_9_LabelColorChanged();

    void button_0_activeChanged();
    void button_1_activeChanged();
    void button_2_activeChanged();
    void button_3_activeChanged();
    void button_4_activeChanged();
    void button_5_activeChanged();
    void button_6_activeChanged();
    void button_7_activeChanged();
    void button_8_activeChanged();
    void button_9_activeChanged();

    void goSecureButton_activeChanged();
    void statusMessageChanged();
    void myCallSignChanged();
    void callSignInsigniaImageChanged();
    void insigniaLabelTextChanged();
    void insigniaLabelStateTextChanged();
    void textMsgDisplayChanged();
    void swipeViewIndexChanged();
    void callDialogVisibleChanged();
    void voltageValueChanged();
    void voltageNotifyColorChanged();
    void networkStatusLabelChanged();
    void networkStatusLabelColorChanged();
    void touchBlock_activeChanged();

    void peer_0_keyPercentageChanged();
    void peer_1_keyPercentageChanged();
    void peer_2_keyPercentageChanged();
    void peer_3_keyPercentageChanged();
    void peer_4_keyPercentageChanged();
    void peer_5_keyPercentageChanged();
    void peer_6_keyPercentageChanged();
    void peer_7_keyPercentageChanged();
    void peer_8_keyPercentageChanged();
    void peer_9_keyPercentageChanged();

    void lockScreen_activeChanged();
    void lockScreenPinCodeChanged();
    void camoScreen_activeChanged();
    void vaultScreen_activeChanged();
    void vaultScreenNotifyTextChanged();
    void vaultScreenNotifyColorChanged();
    void vaultScreenNotifyTextColorChanged();
    void wifiStatusTextChanged();
    void wifiNetworksChanged();
    void wifiNotifyTextChanged();
    void wifiNotifyColorChanged();
    void aboutTextContentChanged();
    void deepSleepEnabledChanged();
    void lteEnabledChanged();
    void plmnChanged();
    void taChanged();
    void gcChanged();
    void scChanged();
    void acChanged();
    void rssiChanged();
    void rsrqChanged();
    void rsrpChanged();
    void snrChanged();
    void powerOffVisibleChanged();
    void mainColorChanged();
    void highColorChanged();
    void dimColorChanged();
    void nightModeEnabledChanged();


};

#endif // ENGINECLASS_H
