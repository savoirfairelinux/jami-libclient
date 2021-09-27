/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// JamiStrings as a singleton is to provide global strings entries.
pragma Singleton

import QtQuick

import net.jami.Helpers 1.1

Item {
    readonly property string appTitle: "Jami" + (UpdateManager.isCurrentVersionBeta()
                                                 ? " (BETA)" : "")
    readonly property string httpUserAgentName: "jami-qt"

    // Misc
    property string accept: qsTr("Accept")
    property string acceptAudio: qsTr("Accept in audio")
    property string acceptVideo: qsTr("Accept in video")
    property string refuse: qsTr("Refuse")
    property string endCall: qsTr("End call")
    property string incomingAudioCallFrom: qsTr("Incoming audio call from {}")
    property string incomingVideoCallFrom: qsTr("Incoming video call from {}")
    property string contactSearchConversation: qsTr("Find a user or search for a conversation")
    property string contactSearchInvitations: qsTr("Search your invitations")
    property string invitations: qsTr("Invitations")
    property string description: qsTr("Jami is free software for universal communication which respects the freedoms and the privacy of its users.")
    property string contactLeft: qsTr("You are viewing a conversation where all participants other than you have left. New interactions will not be possible.")
    property string newConversation: qsTr("Start new conversation")

    // AboutPopUp
    property string version: qsTr("Version") + (UpdateManager.isCurrentVersionBeta() ? " (BETA)" : "")
    property string companyDeclarationYear: "© 2015-2021 Savoir-faire Linux Inc."
    property string slogan: "Taranis"
    property string declaration: qsTr("Jami is a free software for universal communication which respects the freedom and privacy of its users.")
    property string credits: qsTr("Credits")

    // AccountComboBox
    property string displayQRCode: qsTr("Display QR code")
    property string openSettings: qsTr("Open settings")
    property string closeSettings: qsTr("Close settings")

    // AccountMigrationDialog
    property string authenticationRequired: qsTr("Authentication required")
    property string migrationReason: qsTr("Your session has expired or been revoked on this device. Please enter your password.")
    property string jamsServer: qsTr("JAMS server")
    property string authenticate: qsTr("Authenticate")
    property string deleteAccount: qsTr("Delete account")
    property string inProgress: qsTr("In progress…")
    property string authenticationFailed: qsTr("Authentication failed")
    property string password: qsTr("Password")
    property string username: qsTr("Username")
    property string alias: qsTr("Alias")

    // AdvancedCallSettings
    property string callSettings: qsTr("Call Settings")
    property string allowCallsUnknownContacs: qsTr("Allow incoming calls from unknown contacts")
    property string rendezVous: qsTr("Convert your account into a rendezvous point")
    property string autoAnswerCalls: qsTr("Automatically answer calls")
    property string enableCustomRingtone: qsTr("Enable custom ringtone")
    property string selectCustomRingtone: qsTr("Select custom ringtone")
    property string addCustomRingtone: qsTr("Add a custom ringtone")
    property string selectNewRingtone: qsTr("Select a new ringtone")

    // AdvancedChatSettings
    property string chatSettings: qsTr("Chat Settings")
    property string enableReadReceipts: qsTr("Enable read receipts")
    property string enableReadReceiptsTooltip: qsTr("Send and receive receipts indicating that a message have been displayed")

    // AdvancedVoiceMailSettings
    property string voiceMail: qsTr("Voicemail")
    property string voiceMailDialCode: qsTr("Voicemail dial code")

    // AdvancedSIPSecuritySettings && AdvancedJamiSecuritySettings
    property string security: qsTr("Security")
    property string encryptMediaStream: qsTr("Encrypt media streams (SRTP)")
    property string enableSDES: qsTr("Enable SDES key exchange")
    property string fallbackRTP: qsTr("Allow fallback on RTP")
    property string encryptNegotiation: qsTr("Encrypt negotiation (TLS)")
    property string caCertificate: qsTr("CA certificate")
    property string userCertificate: qsTr("User certificate")
    property string privateKey: qsTr("Private key")
    property string privateKeyPassword: qsTr("Private key password")
    property string verifyCertificatesServer: qsTr("Verify certificates for incoming TLS connections")
    property string verifyCertificatesClient: qsTr("Verify server TLS certificates")
    property string tlsRequireConnections: qsTr("Require certificate for incoming TLS connections")
    property string tlsProtocol: qsTr("TLS protocol method")
    property string audioDeviceSelector: qsTr("Audio input device selector")
    property string tlsServerName: qsTr("TLS server name")
    property string negotiationTimeOut: qsTr("Negotiation timeout (seconds)")
    property string selectPrivateKey: qsTr("Select a private key")
    property string selectUserCert: qsTr("Select a user certificate")
    property string selectCACert: qsTr("Select a CA certificate")

    // AdvancedConnectivitySettings
    property string connectivity: qsTr("Connectivity")
    property string autoRegistration: qsTr("Auto Registration After Expired")
    property string registrationExpirationTime: qsTr("Registration expiration time (seconds)")
    property string networkInterface: qsTr("Network interface")
    property string useUPnP: qsTr("Use UPnP")
    property string useTURN: qsTr("Use TURN")
    property string turnAdress: qsTr("TURN address")
    property string turnUsername: qsTr("TURN username")
    property string turnPassword: qsTr("TURN password")
    property string turnRealm: qsTr("TURN Realm")
    property string useSTUN: qsTr("Use STUN")
    property string stunAdress: qsTr("STUN address")

    // AdvancedPublicAddressSettings
    property string allowIPAutoRewrite: qsTr("Allow IP Auto Rewrite")
    property string publicAddress: qsTr("Public address")
    property string useCustomAddress: qsTr("Use custom address and port")
    property string address: qsTr("Address")
    property string port: qsTr("Port")

    // AdvancedMediaSettings
    property string media: qsTr("Media")
    property string enableVideo: qsTr("Enable video")
    property string videoCodecs: qsTr("Video Codecs")
    property string audioCodecs: qsTr("Audio Codecs")

    // AdvancedNameServerSettings
    property string nameServer: qsTr("Name Server")

    // AdvancedSDPSettings
    property string sdpSettingsTitle: qsTr("SDP Session Negotiation (ICE Fallback)")
    property string sdpSettingsSubtitle: qsTr("Only used during negotiation in case ICE is not supported")
    property string audioRTPMinPort: qsTr("Audio RTP minimum Port")
    property string audioRTPMaxPort: qsTr("Audio RTP maximum Port")
    property string videoRTPMinPort: qsTr("Video RTP minimum Port")
    property string videoRTPMaxPort: qsTr("Video RTP maximum port")

    // AdvancedOpenDHTSettings
    property string enablePeerDiscovery: qsTr("Enable local peer discovery")
    property string tooltipPeerDiscovery: qsTr("Connect to other DHT nodes advertising on your local network.")
    property string openDHTConfig: qsTr("OpenDHT Configuration")
    property string enableProxy: qsTr("Enable proxy")
    property string proxyAddress: qsTr("Proxy address")
    property string bootstrap: qsTr("Bootstrap")

    // SettingsHeader
    property string back: qsTr("Back")
    property string accountSettingsTitle: qsTr("Account Settings")
    property string accountSettingsMenuTitle: qsTr("Account")
    property string generalSettingsTitle: qsTr("General")
    property string pluginSettingsTitle: qsTr("Plugin")
    property string avSettingsTitle: qsTr("Audio and Video Settings")
    property string avSettingsMenuTitle: qsTr("Audio/Video")

    // AudioSettings
    property string audio: qsTr("Audio")
    property string microphone: qsTr("Microphone")
    property string selectAudioInputDevice: qsTr("Select audio input device")
    property string outputDevice: qsTr("Output device")
    property string selectAudioOutputDevice: qsTr("Select audio output device")
    property string ringtoneDevice: qsTr("Ringtone device")
    property string selectRingtoneOutputDevice: qsTr("Select ringtone output device")
    property string audioManager: qsTr("Audio manager")

    // VideoSettings
    property string video: qsTr("Video")
    property string selectVideoDevice: qsTr("Select video device")
    property string device: qsTr("Device")
    property string resolution: qsTr("Resolution")
    property string selectVideoResolution: qsTr("Select video resolution")
    property string fps: qsTr("Frames per second")
    property string selectFPS: qsTr("Select video frame rate (frames per second)")
    property string enableHWAccel: qsTr("Enable hardware acceleration")
    property string previewUnavailable: qsTr("Preview unavailable")
    property string screenSharing: qsTr("Screen Sharing")
    property string selectScreenSharingFPS: qsTr("Select screen sharing frame rate (frames per second)")

    // BackupKeyPage
    property string  backupAccountInfos: qsTr("Your account only exists on this device. " +
                                            "If you lose your device or uninstall the application, " +
                                            "your account will be deleted and can not be recovered. " +
                                            "You can backup your account now or later.")
    property string backupAccountHere: qsTr("Backup account here")
    property string backupAccount: qsTr("Backup your account!")
    property string backupAccountBtn: qsTr("Backup account")
    property string skip: qsTr("Skip")
    property string success: qsTr("Success")
    property string error: qsTr("Error")
    property string neverShowAgain: qsTr("Never show me this again")
    property string recommended: qsTr("Recommended")
    property string jamiArchiveFiles: qsTr("Jami archive files")
    property string allFiles: qsTr("All files")

    // BannedItemDelegate
    property string reinstateContact: qsTr("Reinstate as contact")
    property string name: qsTr("name")
    property string identifier: qsTr("Identifier")

    // CallOverlay
    property string isRecording: qsTr("is recording")
    property string areRecording: qsTr("are recording")
    property string peerStoppedRecording: qsTr("Peer stopped recording")
    property string isCallingYou: qsTr("is calling you")
    property string mute: qsTr("Mute")
    property string unmute: qsTr("Unmute")
    property string hangup: qsTr("End call")
    property string pauseCall: qsTr("Pause call")
    property string resumeCall: qsTr("Resume call")
    property string pauseVideo: qsTr("Pause video")
    property string resumeVideo: qsTr("Resume video")
    property string addParticipant: qsTr("Add participant")
    property string addParticipants: qsTr("Add participants")
    property string chat: qsTr("Chat")
    property string moreOptions: qsTr("More options")
    property string mosaic: qsTr("Mosaic")
    property string participantMicIsStillMuted: qsTr("Participant is still muted on their device")
    property string mutedLocally: qsTr("You are still muted on your device")
    property string participantModIsStillMuted: qsTr("You are still muted by moderator")
    property string mutedByModerator: qsTr("You are muted by a moderator")
    property string moderator: qsTr("Moderator")
    property string host: qsTr("Host")
    property string bothMuted: qsTr("Local and Moderator muted")
    property string moderatorMuted: qsTr("Moderator muted")
    property string notMuted: qsTr("Not muted")

    // LineEditContextMenu
    property string copy: qsTr("Copy")
    property string cut: qsTr("Cut")
    property string paste: qsTr("Paste")

    // ConversationContextMenu
    property string startVideoCall: qsTr("Start video call")
    property string startAudioCall: qsTr("Start audio call")
    property string clearConversation: qsTr("Clear conversation")
    property string removeConversation: qsTr("Remove conversation")
    property string removeContact: qsTr("Remove contact")
    property string blockContact: qsTr("Block contact")
    property string contactDetails: qsTr("Contact details")

    // CallViewContextMenu
    property string hold: qsTr("Hold")
    property string sipInputPanel: qsTr("Sip input panel")
    property string transferCall: qsTr("Transfer call")
    property string stopRec: qsTr("Stop recording")
    property string startRec: qsTr("Start recording")
    property string exitFullScreen: qsTr("Exit full screen")
    property string fullScreen: qsTr("Full screen")
    property string shareScreen: qsTr("Share screen")
    property string stopSharing: qsTr("Stop sharing screen or file")
    property string shareScreenArea: qsTr("Share screen area")
    property string shareFile: qsTr("Share file")
    property string selectShareMethod: qsTr("Select sharing method")
    property string viewPlugin: qsTr("View plugin")
    property string noVideoDevice: qsTr("No video device")
    property string notAvailable: qsTr("N/A")
    property string lowerHand: qsTr("Lower hand")
    property string raiseHand: qsTr("Raise hand")

    // Chatview header
    property string hideChatView: qsTr("Hide chat view")
    property string placeAudioCall: qsTr("Place audio call")
    property string placeVideoCall: qsTr("Place video call")
    property string showPlugins: qsTr("Show available plugins")
    property string addToConversations: qsTr("Add to conversations")

    // Chatview footer
    property string jumpToLatest: qsTr("Jump to latest")
    property string typeIndicatorSingle: qsTr("{} is typing…")
    property string typeIndicatorPlural: qsTr("{} are typing…")
    property string typeIndicatorMax: qsTr("Several people are typing…")
    property string typeIndicatorAnd: qsTr(" and ")

    // ConnectToAccountManager
    property string enterJAMSURL: qsTr("Enter Jami Account Management Server (JAMS) URL")
    property string required: qsTr("Required")
    property string jamiManagementServerURL: qsTr("Jami Account Management Server URL")
    property string jamsCredentials: qsTr("Enter your JAMS credentials")
    property string connect: qsTr("Connect")
    property string creatingAccount: qsTr("Creating account…")
    property string backToWelcome: qsTr("Back to welcome page")

    // CreateAccountPage
    property string chooseName: qsTr("Choose name")
    property string chooseUsername: qsTr("Choose username")
    property string createPassword: qsTr("Encrypt account with password")
    property string createRV: qsTr("Create a rendezvous point")
    property string createAccount: qsTr("Create account")
    property string confirmPassword: qsTr("Confirm password")
    property string notePasswordRecovery: qsTr("Choose a password to encrypt your account on this device. Note that the password cannot be recovered.")
    property string optional: qsTr("Optional")
    property string chooseUsernameForAccount: qsTr("Choose a username for your account")
    property string chooseUsernameForRV: qsTr("Choose a name for your rendezvous point")
    property string chooseAName: qsTr("Choose a name")
    property string chooseYourUserName: qsTr("Choose your username")
    property string invalidName: qsTr("Invalid name")
    property string invalidUsername: qsTr("Invalid username")
    property string nameAlreadyTaken: qsTr("Name already taken")
    property string usernameAlreadyTaken: qsTr("Username already taken")

    // CreateSIPAccountPage
    property string proxy: qsTr("Proxy")
    property string server: qsTr("Server")
    property string createSIPAccount: qsTr("Create SIP account")
    property string configureExistingSIP: qsTr("Configure an existing SIP account")

    // CurrentAccountSettings && AdvancedSettings
    property string backupSuccessful: qsTr("Backup successful")
    property string backupFailed: qsTr("Backup failed")
    property string changePasswordSuccess: qsTr("Password changed successfully")
    property string changePasswordFailed: qsTr("Password change failed")
    property string setPasswordSuccess: qsTr("Password set successfully")
    property string setPasswordFailed: qsTr("Password set failed")
    property string changePassword: qsTr("Change password")
    property string setPassword: qsTr("Set password")
    property string setAPassword: qsTr("Set password")
    property string changeCurrentPassword: qsTr("Change current password")
    property string tipBackupAccount: qsTr("Backup account to a .gz file")
    property string tipAdvancedSettingsDisplay: qsTr("Display advanced settings")
    property string tipAdvancedSettingsHide: qsTr("Hide advanced settings")
    property string enableAccount: qsTr("Enable account")

    // NameRegistrationDialog
    property string setUsername: qsTr("Set username")
    property string registeringName: qsTr("Registering name")

    // JamiUserIdentity
    property string registerAUsername: qsTr("Register a username")
    property string registerUsername: qsTr("Register username")

    // LinkedDevices
    property string tipLinkNewDevice: qsTr("Link a new device to this account")
    property string linkAnotherDevice: qsTr("Link another device")
    property string removeDevice: qsTr("Remove Device")
    property string sureToRemoveDevice: qsTr("Are you sure you wish to remove this device?")
    property string linkedDevices: qsTr("Linked Devices")
    property string yourPinIs: qsTr("Your PIN is:")
    property string linkDeviceNetWorkError: qsTr("Error connecting to the network.\nPlease try again later.")

    // BannedContacts
    property string tipBannedContactsDisplay: qsTr("Display banned contacts")
    property string tipBannedContactsHide: qsTr("Hide banned contacts")

    // DeleteAccountDialog
    property string confirmDeleteQuestion: qsTr("Do you really want to delete this account?")
    property string deleteAccountInfos: qsTr("If your account has not been backed up or added to another device, your account and registered name will be irrevocably lost.")

    // DeviceItemDelegate
    property string saveNewDeviceName: qsTr("Save new device name")
    property string editDeviceName: qsTr("Edit device name")
    property string unlinkDevice: qsTr("Unlink device from account")

    // SystemSettings
    property string selectFolder: qsTr("Select a folder")
    property string enableNotifications: qsTr("Enable notifications")
    property string enableDarkTheme: qsTr("Enable dark theme")
    property string keepMinimized: qsTr("Keep minimized on close")
    property string tipRunStartup: qsTr("Run application on system startup")
    property string runStartup: qsTr("Launch at startup")
    property string downloadFolder: qsTr("Downloads directory")
    property string tipChooseDownloadFolder: qsTr("Choose download directory")
    property string recordCall: qsTr("Record call")

    // ChatviewSettings
    property string chatview: qsTr("Chatview")
    property string enableTypingIndicator: qsTr("Enable typing indicators")
    property string displayHyperlinkPreviews: qsTr("Display hyperlink previews in the chatview")

    // File transfer settings
    property string fileTransfer: qsTr("File transfer")
    property string allowFromUntrusted: qsTr("Allow incoming files from unknown contacts")
    property string autoAcceptFiles: qsTr("Automatically accept incoming files")
    property string acceptTransferBelow: qsTr("Accept transfer limit")
    property string acceptTransferTooltip: qsTr("in MB, 0 = unlimited")

    // JamiUserIdentity settings
    property string register: qsTr("Register")
    property string incorrectPassword: qsTr("Incorrect password")
    property string networkError: qsTr("Network error")
    property string somethingWentWrong: qsTr("Something went wrong")

    // Context Menu
    property string saveFile: qsTr("Save file")
    property string openLocation: qsTr("Open location")

    // Updates
    property string betaInstall: qsTr("Install beta version")
    property string checkForUpdates: qsTr("Check for updates now")
    property string enableAutoUpdates: qsTr("Enable/Disable automatic updates")
    property string tipAutoUpdate: qsTr("toggle automatic updates")
    property string updatesTitle: qsTr("Updates")
    property string updateDialogTitle: qsTr("Update")
    property string updateFound: qsTr("A new version of Jami was found\n Would you like to update now?")
    property string updateNotFound: qsTr("No new version of Jami was found")
    property string updateCheckError: qsTr("An error occured when checking for a new version")
    property string updateNetworkError: qsTr("Network error")
    property string updateSSLError: qsTr("SSL error")
    property string updateDownloadCanceled: qsTr("Installer download canceled")
    property string updateDownloading: "Downloading"
    property string confirmBeta: qsTr("This will uninstall your current Release version and you can always download the latest Release version on our website")
    property string networkDisconnected: qsTr("Network disconnected")
    property string genericError: qsTr("Something went wrong")

    //Troubleshoot Settings
    property string troubleshootTitle: qsTr("Troubleshoot")
    property string troubleshootButton: qsTr("Open logs")
    property string troubleshootText: qsTr("Get logs")

    // Recording Settings
    property string tipRecordFolder: qsTr("Select a record directory")

    // KeyboardShortCutTable
    property string keyboardShortcutTableWindowTitle: qsTr("Keyboard Shortcut Table")
    property string generalKeyboardShortcuts: qsTr("General")
    property string conversationKeyboardShortcuts: qsTr("Conversation")
    property string callKeyboardShortcuts: qsTr("Call")
    property string settingsKeyboardShortcuts: qsTr("Settings")

    // View Logs
    property string logsViewTitle: qsTr("Debug")
    property string logsViewShowStats: qsTr("Show Stats")
    property string logsViewStart: qsTr("Start")
    property string logsViewStop: qsTr("Stop")
    property string logsViewCopy: qsTr("Copy")
    property string logsViewReport: qsTr("Report Bug")
    property string logsViewClear: qsTr("Clear")
    property string logsViewCancel: qsTr("Cancel")
    property string logsViewCopied: qsTr("Copied to clipboard!")
    property string logsViewDisplay: qsTr("Receive Logs")

    // ImportFromBackupPage
    property string archive: qsTr("Archive")
    property string openFile: qsTr("Open file")
    property string importAccountArchive: qsTr("Create your account from a backup")
    property string importAccountExplanation: qsTr("You can obtain an archive by clicking on \"Backup account\" " +
                                                   "in the account settings. " +
                                                   "This will create a .gz file on your device.")
    property string connectFromBackup: qsTr("Restore an account from backup")
    property string generatingAccount: qsTr("Generating account…")
    property string importFromBackup: qsTr("Import from backup")

    // ImportFromDevicePage
    property string mainAccountPassword: qsTr("Enter Jami account password")
    property string enterPIN: qsTr("Enter the PIN from another configured Jami account. " +
                                "Use the \"Link Another Device\" feature to obtain a PIN.")
    property string connectFromAnotherDevice: qsTr("Link device")
    property string pin: qsTr("PIN")

    // LinkDevicesDialog
    property string pinTimerInfos: qsTr("The PIN and the account password should be entered in your device within 10 minutes.")
    property string close: qsTr("Close")
    property string enterAccountPassword: qsTr("Enter your account password")
    property string addDevice: qsTr("Add Device")

    // PasswordDialog
    property string enterPassword: qsTr("Enter the password")
    property string enterCurrentPassword: qsTr("Enter current password")
    property string confirmRemoval: qsTr("Enter this account's password to confirm the removal of this device")
    property string enterNewPassword: qsTr("Enter new password")
    property string confirmNewPassword: qsTr("Confirm new password")
    property string change: qsTr("Change")
    property string confirm: qsTr("Confirm")
    property string exportAccount: qsTr("Export")

    // PhotoBoothView
    property string chooseAvatarImage: qsTr("Choose a picture as avatar")
    property string importFromFile: qsTr("Import avatar from image file")
    property string stopTakingPhoto: qsTr("Stop taking photo")
    property string clearAvatar: qsTr("Clear avatar image")
    property string takePhoto: qsTr("Take photo")

    // PluginSettingsPage
    property string enable: qsTr("Enable")

    // PluginListPreferencesView
    property string reset: qsTr("Reset")
    property string selectPluginInstall: qsTr("Select a plugin to install")
    property string installPlugin: qsTr("Install plugin")

    // PluginItemDelegate
    property string showHidePrefs: qsTr("Display or hide preferences")

    // PluginListSettingsView
    property string addNewPlugin: qsTr("Add new plugin")
    property string goBackToPluginsList: qsTr("Go back to plugins list")

    // PreferenceItemDelegate
    property string selectFile: qsTr("Select a file")
    property string select: qsTr("Select")
    property string chooseImageFile: qsTr("Choose image file")

    // ProfilePage
    property string profileSharedWithContacts: qsTr("Profile is only shared with contacts")
    property string saveProfile: qsTr("Save profile")
    property string enterYourName: qsTr("Enter your name")
    property string enterRVName: qsTr("Enter the rendezvous point's name")
    property string generatingRV: qsTr("Creating rendezvous point…")
    property string information: qsTr("Information")

    // RevokeDevicePasswordDialog
    property string confirmRemovalRequest: qsTr("Enter the account password to confirm the removal of this device")

    // SelectScreen
    property string selectScreen: qsTr("Select a screen to share")
    property string allScreens: qsTr("All Screens")
    property string screens: qsTr("Screens")
    property string windows: qsTr("Windows")

    // UserProfile
    property string qrCode: qsTr("QR code")

    // Account QR
    property string accountQr: qsTr("Account QR")

    // WelcomePage
    property string shareInvite: qsTr("This is your Jami username.\nCopy and share it with your friends!")
    property string linkFromAnotherDevice: qsTr("Link this device to an existing account")
    property string importAccountFromOtherDevice: qsTr("Import account from other device")
    property string importAccountFromBackup: qsTr("Import account from backup file")
    property string advancedFeatures: qsTr("Advanced features")
    property string showAdvancedFeatures: qsTr("Show advanced features")
    property string hideAdvancedFeatures: qsTr("Hide advanced features")
    property string connectJAMSServer: qsTr("Connect to a JAMS server")
    property string createFromJAMS: qsTr("Create account from Jami Account Management Server (JAMS)")
    property string addSIPAccount: qsTr("Add a SIP account")
    property string errorCreateAccount: qsTr("Error while creating your account. Check your credentials.")
    property string createNewRV: qsTr("Create new rendezvous point")
    property string createAJamiAccount: qsTr("Create a Jami account")
    property string createNewJamiAccount: qsTr("Create new Jami account")
    property string createNewSipAccount: qsTr("Create new SIP account")
    property string aboutJami: qsTr("About Jami")
    property string welcomeTo: qsTr("Welcome to")

    // SmartList
    property string clearText: qsTr("Clear Text")
    property string conversations: qsTr("Conversations")
    property string searchResults: qsTr("Search Results")

    // SmartList context menu
    property string declineContactRequest: qsTr("Decline contact request")
    property string acceptContactRequest: qsTr("Accept contact request")

    // Update settings
    property string update: qsTr("Automatically check for updates")

    // Generic dialog options
    property string optionOk: qsTr("Ok")
    property string optionCancel: qsTr("Cancel")
    property string optionUpgrade: qsTr("Upgrade")
    property string optionLater: qsTr("Later")
    property string optionDelete: qsTr("Delete")

    // Conference moderation
    property string setModerator: qsTr("Set moderator")
    property string unsetModerator: qsTr("Unset moderator")
    property string muteParticipant: qsTr("Mute")
    property string unmuteParticipant: qsTr("Unmute")
    property string maximizeParticipant: qsTr("Maximize")
    property string minimizeParticipant: qsTr("Minimize")
    property string hangupParticipant: qsTr("Hangup")
    property string localMuted: qsTr("Local muted")

    // Settings moderation
    property string conferenceModeration: qsTr("Conference moderation")
    property string defaultModerators: qsTr("Default moderators")
    property string enableLocalModerators: qsTr("Enable local moderators")
    property string enableAllModerators: qsTr("Make all participants moderators")
    property string addDefaultModerator: qsTr("Add default moderator")
    property string removeDefaultModerator: qsTr("Remove default moderator")

    // Daemon reconnection
    property string reconnectDaemon: qsTr("Trying to reconnect to the Jami daemon (jamid)…")
    property string reconnectionFailed: qsTr("Could not re-connect to the Jami daemon (jamid).\nJami will now quit.")

    // Is Swarm
    property string isSwarm: qsTr("Is swarm:")
    property string trueStr: qsTr("True")
    property string falseStr: qsTr("False")

    // Message view
    property string addEmoji: qsTr("Add emoji")
    property string sendFile: qsTr("Send file")
    property string leaveAudioMessage: qsTr("Leave audio message")
    property string leaveVideoMessage: qsTr("Leave video message")
    property string send: qsTr("Send")
    property string remove: qsTr("Remove")
    property string writeTo: qsTr("Write to %1")

    // Invitation View
    property string invitationViewSentRequest: qsTr("%1 has sent you a request for a conversation.")
    property string invitationViewJoinConversation: qsTr("Hello,\nWould you like to join the conversation?")
    property string invitationViewAcceptedConversation: qsTr("You have accepted\nthe conversation request")
    property string invitationViewWaitingForSync: qsTr("Waiting until %1\nconnects to synchronize the conversation.")
}
