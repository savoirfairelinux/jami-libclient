"use strict"

/* Constants used at several places */
// scrollDetectionThresh represents the number of pixels a user can scroll
// without disabling the automatic go-back-to-bottom when a new message is
// received
const scrollDetectionThresh = 70
// printHistoryPart loads blocks of messages. Each block contains
// scrollBuffer messages
const scrollBuffer = 20
// The first time a conversation is loaded, the lazy loading system makes
// sure at least initialScrollBufferFactor screens of messages are loaded
const initialScrollBufferFactor = 3
// Some signal like the onscrolled signals are debounced so that the their
// assigned function isn't fired too often
const debounceTime = 200

/* Buffers */
// current index in the history buffer
var historyBufferIndex = 0
// buffer containing the conversation's messages
var historyBuffer = []

// If we use qt
var use_qt = false

var is_swarm = false

if (navigator.userAgent == "jami-qt") {
    use_qt = true
}

/* We retrieve refs to the most used navbar and message bar elements for efficiency purposes */
/* NOTE: use getElementById when possible (more readable and efficient) */
const addToConversationsButton = document.getElementById("addToConversationsButton")
const placeAudioCallButton = document.getElementById("placeAudioCallButton")
const backButton = document.getElementById("backButton")
const placeCallButton = document.getElementById("placeCallButton")
const pluginsButton = document.getElementById("pluginsButton")
const unbanButton = document.getElementById("unbanButton")
const acceptButton = document.getElementById("acceptButton")
const refuseButton = document.getElementById("refuseButton")
const blockButton = document.getElementById("blockButton")

const callButtons = document.getElementById("callButtons")
const sendButton = document.getElementById("sendButton")
sendButton.style.display = "none"
const optionsButton = document.getElementById("optionsButton")
const backToBottomBtn = document.getElementById("back_to_bottom_button")
const backToBottomBtnContainer = document.getElementById("back_to_bottom_button_container")
const sendFileButton = document.getElementById("sendFileButton")
const videoRecordButton = document.getElementById("videoRecordButton")
const audioRecordButton = document.getElementById("audioRecordButton")
const aliasField = document.getElementById("nav-contactid-alias")
const bestIdField = document.getElementById("nav-contactid-bestId")
const idField = document.getElementById("nav-contactid")
const messageBar = document.getElementById("sendMessage")
const messageBarInput = document.getElementById("message")
const addToConvButton = document.getElementById("addToConversationsButton")
const invitation = document.getElementById("invitation")
invitation.style.display = "none"

const inviteImage = document.getElementById("invite_image")

const navbar = document.getElementById("navbar")
const emojiBtn = document.getElementById("emojiButton")
const invitationText = document.getElementById("invitation_text")
const joinText = document.getElementById("join_text")
const noteText = document.getElementById("note_text")

var messages = document.getElementById("messages")
var sendContainer = document.getElementById("data_transfer_send_container")
var wrapperOfNavbar = document.getElementById("wrapperOfNavbar")

/* States: allows us to avoid re-doing something if it isn't meaningful */
var displayLinksEnabled = true
var hoverBackButtonAllowed = true
var hasInvitation = false
var isTemporary = false
var isBanned = false
var isAccountEnabled = true
var isInitialLoading = false
var imagesLoadingCounter = 0
var canLazyLoad = false

// ctrl + click on link will crash QtWebEngineProcess
document.body.onclick = function(e) {
    if (e.ctrlKey) {
        return false
    }
}

// same for mouse middle button
document.body.onauxclick = function(e) {
    if (e.which === 2) {
        return false
    }
}

/* String sipmle format prototype*/
String.prototype.format = function() {
    var a = this
    for (var k in arguments) {
        a = a.replace("{" + k + "}", arguments[k])
    }
    return a
}

/* Set the default target to _self and handle with QWebEnginePage::acceptNavigationRequest */
var linkifyOptions = {}
if (use_qt) {
    messageBarInput.onpaste = pasteKeyDetected
    linkifyOptions = {
        attributes: null,
        className: "linkified",
        defaultProtocol: "http",
        events: null,
        format: function (value, type) {
            return value
        },
        formatHref: function (href, type) {
            return href
        },
        ignoreTags: [],
        nl2br: false,
        tagName: "a",
        target: {
            url: "_self"
        },
        validate: true
    }
    new QWebChannel(qt.webChannelTransport, function (channel) {
        window.jsbridge = channel.objects.jsbridge

        // connect to a signal
        window.jsbridge.setSendMessageContentRequest.connect(function(content) {
            setSendMessageContent(content)
        })
    })
}

/* i18n manager */
var i18n = null

/* i18n string data for client-qt*/
var i18nStringData

/* exported init_i18n */
function init_i18n(data) {
    if (use_qt) {
        window.jsbridge.parseI18nData(function(stringData) {
            i18nStringData = stringData
            reset_message_bar_input("")
            set_titles()
        })
    } else {
        if (data === undefined) {
            i18n = new Jed({ locale_data: { "messages": { "": {} } } }) // eslint-disable-line no-undef
        } else {
            var domain = {
                "" : {
                    // Domain name
                    "domain" : "messages",
                }
            }
            for  (var key in data) {
                domain[key] = [data[key]]
            }
            i18n = new Jed({ locale_data: { "messages": domain } })
        }
        reset_message_bar_input("")
        set_titles()
    }
}

/* exported init_picker */
function init_picker(dark) {
    const picker = new EmojiButton({
        theme: dark? "dark" : "light"
    })
    picker.on("emoji", emoji => {
        messageBarInput.value += emoji.emoji
    })
    emojiBtn.addEventListener("click", () => {
        picker.togglePicker(emojiBtn)
    })
}

/* exported set_is_swarm */
function set_is_swarm(value) {
    is_swarm = value
}

function set_titles() {
    if (use_qt){
        backButton.title = i18nStringData["Hide chat view"]
        placeCallButton.title = i18nStringData["Place video call"]
        pluginsButton.title = i18nStringData["Show available plugins"]
        placeAudioCallButton.title = i18nStringData["Place audio call"]
        addToConversationsButton.title = i18nStringData["Add to conversations"]
        unbanButton.title = i18nStringData["Unban contact"]
        sendButton.title = i18nStringData["Send"]
        optionsButton.title = i18nStringData["Options"]
        backToBottomBtn.innerHTML = `${i18nStringData["Jump to latest"]} &#9660;`
        sendFileButton.title = i18nStringData["Send file"]
        emojiBtn.title = i18nStringData["Add emoji"]
        videoRecordButton.title = i18nStringData["Leave video message"]
        audioRecordButton.title = i18nStringData["Leave audio message"]
        acceptButton.title = i18nStringData["Accept"]
        refuseButton.title = i18nStringData["Refuse"]
        blockButton.title = i18nStringData["Block"]
        acceptButton.innerHTML = i18nStringData["Accept"]
        refuseButton.innerHTML = i18nStringData["Refuse"]
        blockButton.innerHTML = i18nStringData["Block"]
    } else {
        backButton.title = i18n.gettext("Hide chat view")
        placeCallButton.title = i18n.gettext("Place video call")
        pluginsButton.title = i18n.gettext("Show available plugins")
        placeAudioCallButton.title = i18n.gettext("Place audio call")
        addToConversationsButton.title = i18n.gettext("Add to conversations")
        unbanButton.title = i18n.gettext("Unban contact")
        sendButton.title = i18n.gettext("Send")
        optionsButton.title = i18n.gettext("Options")
        backToBottomBtn.innerHTML = `${i18n.gettext("Jump to latest")} &#9660;`
        sendFileButton.title = i18n.gettext("Send file")
        emojiBtn.title = i18n.gettext("Add emoji")
        videoRecordButton.title = i18n.gettext("Leave video message")
        audioRecordButton.title = i18n.gettext("Leave audio message")
        acceptButton.title = i18n.gettext("Accept")
        refuseButton.title = i18n.gettext("Refuse")
        blockButton.title = i18n.gettext("Block")
        acceptButton.innerHTML = i18n.gettext("Accept")
        refuseButton.innerHTML = i18n.gettext("Refuse")
        blockButton.innerHTML = i18n.gettext("Block")
    }
}

function reset_message_bar_input(name) {
    messageBarInput.placeholder = use_qt ?
        i18nStringData["Write to {0}"].format(name) :
        i18n.gettext("Write to {0}").format(name)
}

function onScrolled_() {
    if (!canLazyLoad)
        return
    // back to bottom button
    if(messages.scrollTop >= messages.scrollHeight - messages.clientHeight - scrollDetectionThresh) {
        // fade out
        if (!backToBottomBtn.classList.contains("fade")) {
            backToBottomBtn.classList.add("fade")
            backToBottomBtn.removeAttribute("onclick")
        }
    } else {
        // fade in
        if (backToBottomBtn.classList.contains("fade")) {
            backToBottomBtn.style.visibility = "visible"
            backToBottomBtnContainer.style.visibility = "visible"
            backToBottomBtn.classList.remove("fade")
            backToBottomBtn.onclick = back_to_bottom
        }
    }

    if (messages.scrollTop === 0) {
        if (use_qt) {
            window.jsbridge.loadMessages(scrollBuffer)
        } else {
            window.prompt(`LOAD_MESSAGES:${scrollBuffer}`)
        }
    }
}

const debounce = (fn, time) => {
    let timeout

    return function () {
        const functionCall = () => fn.apply(this, arguments)

        clearTimeout(timeout)
        timeout = setTimeout(functionCall, time)
    }
}

/* exported onScrolled */
var onScrolled = debounce(onScrolled_, debounceTime)

/**
 * Generic wrapper. Execute passed function keeping scroll position identical.
 *
 * @param func function to execute
 * @param args parameters as array
 */
function exec_keeping_scroll_position(func, args) {
    var atEnd = messages.scrollTop >= messages.scrollHeight - messages.clientHeight - scrollDetectionThresh
    func(...args)
    if (atEnd) {
        messages.scrollTop = messages.scrollHeight
    }
}

/**
 * Reset scrollbar at a given position.
 * @param scroll position at which the scrollbar should be set.
 *               Here position means the number of pixels scrolled,
 *               i.e. scroll = 0 resets the scrollbar at the bottom.
 */
function back_to_scroll(scroll) {
    messages.scrollTop = messages.scrollHeight - scroll
}

/**
 * Reset scrollbar at bottom.
 */
function back_to_bottom() {
    back_to_scroll(0)
}

/**
 * Update common frame between conversations.
 *
 * Whenever the current conversation is switched, information from the navbar
 * and message bar have to be updated to match new contact. This function
 * handles most of the required work (except the showing/hiding the invitation,
 * which is handled by showInvitation()).
 *
 * @param accountEnabled 	whether account is enabled or not
 * @param banned 	whether contact is banned or not
 * @param temporary	whether contact is temporary or not
 * @param alias
 * @param bestId
 */
/* exported update_chatview_frame */
function update_chatview_frame(accountEnabled, banned, temporary, bestname, bestid) {
    /* This function updates lots of things in the navbar and we don't want to
       trigger that many DOM updates. Instead set display to none so DOM is
       updated only once. */
    navbar.style.display = "none"

    hoverBackButtonAllowed = true

    aliasField.innerHTML = (bestname ? bestname : bestid)

    if (bestname) {
        bestIdField.innerHTML = bestid
        idField.classList.remove("oneEntry")
    } else {
        idField.classList.add("oneEntry")
    }

    if (isAccountEnabled !== accountEnabled) {
        isAccountEnabled = accountEnabled
        hideMessageBar(!accountEnabled)
        hideControls(!accountEnabled)
    }

    if (isBanned !== banned) {
        isBanned = banned
        hideMessageBar(banned)

        if (banned) {
            // contact is banned. update navbar and states
            navbar.classList.add("onBannedState")
        } else {
            navbar.classList.remove("onBannedState")
        }
    } else if (isTemporary !== temporary) {
        isTemporary = temporary
        if (temporary) {
            addToConvButton.style.display = "flex"
            messageBarInput.placeholder = use_qt ?
                i18nStringData["Note: an interaction will create a new contact."] :
                i18n.gettext("Note: an interaction will create a new contact.")
        } else {
            addToConvButton.style.display = ""
        }
    }

    if (!temporary) {
        reset_message_bar_input(bestname ? bestname : bestid)
    }

    navbar.style.display = ""
}


/**
 * Hide or show invitation to a conversation.
 *
 * Invitation is hidden if no bestId is passed.
 * Otherwise, invitation div is updated.
 *
 * @param bestName
 * @param bestId
 * @param isSyncing
 */
/* exported showInvitation */
function showInvitation(bestName, bestId, isSyncing) {
    if (!bestId) {
        if (hasInvitation) {
            hasInvitation = false
            invitation.style.display = "none"
            messages.style.visibility = "visible"
        }
        messageBar.style.visibility = "visible"
    } else {
        if (!inviteImage.classList.contains("sender_image")) {
            inviteImage.classList.add("sender_image")
        }
        if (use_qt) {
            if (!inviteImage.classList.contains(`sender_image_${bestId}`)) {
                inviteImage.classList.add(`sender_image_${bestId}`)
            }
        } else {
            const className = `sender_image_${bestId}`.replace(/@/g, "_").replace(/\./g, "_")
            if (!inviteImage.classList.contains(className)) {
                inviteImage.classList.add(className)
            }
        }
        invitationText.innerHTML = (bestName ? bestName : bestId)
            + " "
            + (use_qt
               ? i18nStringData["has sent you a conversation request."]
               : i18n.gettext("has sent you a conversation request."))
            + "<br/>"

        var joinTextValue = (isSyncing ?
            "You have accepted the conversation request." :
            "Hello, would you like to join the conversation?")
        joinText.innerHTML = (use_qt ?
            i18nStringData[joinTextValue] :
            i18n.gettext(joinTextValue))
            + "<br/>"

        noteText.innerHTML = (use_qt ?
            i18nStringData["We are waiting for another device to synchronize the conversation."] :
            i18n.gettext("We are waiting for another device to synchronize the conversation."))
            + "<br/>"

        messages.style.visibility = "hidden"
        hasInvitation = true

        invitation.style.display = "flex"
        invitation.style.visibility = "visible"

        var actions = document.getElementById("actions")
        var quote_img = document.getElementById("quote_img_wrapper")
        if (isSyncing) {
            actions.style.visibility = "collapse"
            invitationText.style.visibility = "hidden"
            quote_img.style.visibility = "collapse"
            noteText.style.visibility = "visible"
        } else {
            actions.style.visibility = "visible"
            invitationText.style.visibility = "visible"
            quote_img.style.visibility = "visible"
            noteText.style.visibility = "collapse"
        }
    }
}

/**
 * Hide or show navbar, and update body top padding accordingly.
 *
 * @param isVisible whether navbar should be displayed or not
 */
/* exported displayNavbar */
function displayNavbar(isVisible) {
    if (isVisible) {
        navbar.classList.remove("hiddenState")
        document.body.style.setProperty("--navbar-size", undefined)
    } else {
        navbar.classList.add("hiddenState")
        document.body.style.setProperty("--navbar-size", "0")
    }
}

/**
 * Hide or show record controls, and update body top padding accordingly.
 *
 * @param isVisible whether navbar should be displayed or not
 */
/* exported displayRecordControls */
function displayRecordControls(isVisible) {
    if (isVisible) {
        audioRecordButton.classList.remove("hiddenState")
        videoRecordButton.classList.remove("hiddenState")
    } else {
        audioRecordButton.classList.add("hiddenState")
        videoRecordButton.classList.add("hiddenState")
    }
}

/**
 * Hide or show plugin controls, and update body top padding accordingly.
 *
 * @param isVisible whether navbar should be displayed or not
 */
/* exported displayPluginControl */
function displayPluginControl(isVisible) {
    if (isVisible) {
        pluginsButton.style.removeProperty("display")
    } else {
        pluginsButton.style.setProperty("display", "none")
    }
}


/**
 * Hide or show message bar, and update body bottom padding accordingly.
 *
 * @param hide whether message bar should be displayed or not
 */
/* exported hideMessageBar */
function hideMessageBar(hide) {
    if (hide) {
        messageBar.classList.add("hiddenState")
        document.body.style.setProperty("--messagebar-size", "0")
    } else {
        messageBar.classList.remove("hiddenState")
        document.body.style.removeProperty("--messagebar-size")
    }
}

/**
 * Hide or show call buttons
 *
 * @param hide whether the buttons should be hidden
 */
function hideControls(hide) {
    if (hide) {
        placeAudioCallButton.style.display = "none"
        placeCallButton.style.display = "none"
    } else {
        placeAudioCallButton.style.display = ""
        placeCallButton.style.display = ""
    }
}

/* exported setDisplayLinks */
function setDisplayLinks(display) {
    displayLinksEnabled = display
}

/**
 * This event handler dynamically resizes the message bar depending on the amount of
 * text entered, while adjusting the body paddings so that that the message bar doesn't
 * overlap messages when it grows.
 */
/* exported grow_text_area */
function grow_text_area() {
    exec_keeping_scroll_position(function () {
        var old_height = window.getComputedStyle(messageBar).height
        messageBarInput.style.height = "auto" /* <-- necessary, no clue why */
        messageBarInput.style.height = messageBarInput.scrollHeight + "px"
        var new_height = window.getComputedStyle(messageBar).height

        var msgbar_size = window.getComputedStyle(document.body).getPropertyValue("--messagebar-size")
        var total_size = parseInt(msgbar_size) + parseInt(new_height) - parseInt(old_height)

        document.body.style.setProperty("--messagebar-size", total_size.toString() + "px")

        if (use_qt) {
            window.jsbridge.onComposing(messageBarInput.value.length !== 0)
        } else {
            window.prompt(`ON_COMPOSING:${messageBarInput.value.length !== 0}`)
        }
    }, [])
    checkSendButton()
}

/**
 * This event handler processes keydown events from the message bar. When pressed key is
 * the enter key, send the message unless shift or control was pressed too.
 *
 * @param key the pressed key
 */
/* exported process_messagebar_keydown */
function process_messagebar_keydown(key) {
    key = key || event
    var map = {}

    map[key.keyCode] = key.type == "keydown"
    if (key.ctrlKey && map[13]) {
        messageBarInput.value += "\n"
    }
    if (key.ctrlKey || key.shiftKey) {
        return true
    }
    if (map[13]) {
        sendMessage()
        key.preventDefault()
    }
    return true
}

/* exported clearSenderImages */
function clearSenderImages() {
    var styles = document.head.querySelectorAll("style"),
        i = styles.length

    while (i--) {
        document.head.removeChild(styles[i])
    }
}

/**
 * This event handler adds the hover property back to the "back to welcome view"
 * button.
 *
 * This is a hack. It needs some explanations.
 *
 * Problem: Whenever the "back to welcome view" button is clicked, the webview
 * freezes and the GTK ring welcome view is displayed. While the freeze
 * itself is perfectly fine (probably necessary for good performances), this
 * is a big problem for us when the user opens a chatview again: Since the
 * chatview was freezed, the back button has «remembered» the hover state and
 * still displays the blue background for a small instant. This is a very bad
 * looking artefact.
 *
 * In order to counter this problem, we introduced the following evil mechanism:
 * Whenever a user clicks on the "back to welcome view" button, the hover
 * property is disabled. The hover property stays disabled until the user calls
 * this event handler by hover-ing the button.
 */
/* exported addBackButtonHoverProperty */
function addBackButtonHoverProperty() {
    if (hoverBackButtonAllowed) {
        backButton.classList.add("non-action-button")
    }
}

/**
 * Disable or enable textarea.
 *
 * @param isDisabled whether message bar should be enabled or disabled
 */
/* exported disableSendMessage */
function disableSendMessage(isDisabled) {
    messageBarInput.disabled = isDisabled
}

/*
 * Update timestamps messages.
 */
function updateView() {
    updateTimestamps(messages)
}

setInterval(updateView, 60000)

/* exported addBannedContact */
function addBannedContact() {
    window.prompt("UNBLOCK")
}

/* exported addToConversations */
function addToConversations() {
    window.prompt("ADD_TO_CONVERSATIONS")
}

/* exported placeCall */
function placeCall() {
    window.prompt("PLACE_CALL")
}

/* exported placeAudioCall */
function placeAudioCall() {
    window.prompt("PLACE_AUDIO_CALL")
}

/* exported backToWelcomeView */
function backToWelcomeView() {
    backButton.classList.remove("non-action-button")
    hoverBackButtonAllowed = false
    window.prompt("CLOSE_CHATVIEW")
}

function openPluginHandlersList() {
    var rect = pluginsButton.getBoundingClientRect()
    if (!use_qt) {
        window.prompt(`LIST_PLUGIN_HANDLERS:${rect.left + rect.width / 2}x${rect.bottom}`)
    }
}

/**
 * Transform a date to a string group like "1 hour ago".
 *
 * @param date
 */
function formatDate(date) {
    const seconds = Math.floor((new Date() - date) / 1000)
    var interval = Math.floor(seconds / (3600 * 24))

    if (use_qt) {
        if (interval > 5)
            return date.toLocaleDateString()
        if (interval > 1)
            return "\u200E " + i18nStringData["%d days ago"].format(interval)
        if (interval === 1)
            return "\u200E " + i18nStringData["one day ago"]

        interval = Math.floor(seconds / 3600)
        if (interval > 1)
            return "\u200E " + i18nStringData["%d hours ago"].format(interval)
        if (interval === 1)
            return "\u200E " + i18nStringData["one hour ago"]

        interval = Math.floor(seconds / 60)
        if (interval > 1)
            return "\u200E " + i18nStringData["%d minutes ago"].format(interval)
        return i18nStringData["just now"]
    } else {
        if (interval > 5) {
            return date.toLocaleDateString()
        }

        if (interval > 1) {
            return i18n.sprintf(i18n.gettext("%d days ago"), interval)
        }
        if (interval === 1) {
            return i18n.gettext("one day ago") // what about "yesterday"?
        }

        interval = Math.floor(seconds / 3600)
        if (interval > 1) {
            return i18n.sprintf(i18n.gettext("%d hours ago"), interval)
        }
        if (interval === 1) {
            return i18n.gettext("one hour ago")
        }

        interval = Math.floor(seconds / 60)
        if (interval > 1) {
            return i18n.sprintf(i18n.gettext("%d minutes ago"), interval)
        }

        return i18n.gettext("just now")
    }
}

/**
 * Send content of message bar
 */
function sendMessage() {
    if (use_qt) {
        //send image in sendContainer
        var data_to_send = sendContainer.innerHTML
        var imgSrcExtract = new RegExp("<img src=\"(.*?)\">", "g")
        var fileSrcExtract = new RegExp("<div class=\"file_wrapper\" data-path=\"(.*?)\">", "g")

        var img_src
        while ((img_src = imgSrcExtract.exec(data_to_send)) !== null) {
            window.jsbridge.sendImage(img_src[1])
        }

        var file_src
        while ((file_src = fileSrcExtract.exec(data_to_send)) !== null) {
            window.jsbridge.sendFile(file_src[1])
        }
    }

    var message = messageBarInput.value
    if (message.length > 0) {
        messageBarInput.value = ""
        if (use_qt) {
            window.jsbridge.sendMessage(message)
        } else {
            window.prompt("SEND:" + message)
        }
    }
    grow_text_area()
    reduce_send_container()
}

/* exported acceptInvitation */
function acceptInvitation() {
    if (use_qt) {
        window.jsbridge.acceptInvitation()
    } else {
        window.prompt("ACCEPT")
    }
}
/* exported refuseInvitation */
function refuseInvitation() {
    if (use_qt) {
        window.jsbridge.refuseInvitation()
    } else {
        window.prompt("REFUSE")
    }
}
/* exported blockConversation */
function blockConversation() {
    if (use_qt) {
        window.jsbridge.blockConversation()
    } else {
        window.prompt("BLOCK")
    }
}

/* exported sendFile */
function selectFileToSend() {
    if (use_qt) {
        window.jsbridge.selectFile()
    } else {
        window.prompt("SEND_FILE")
    }
}

/* exported sendFile */
function videoRecord() {
    var rect = videoRecordButton.getBoundingClientRect()
    if (use_qt) {
        window.jsbridge.openVideoRecorder(rect.left + rect.width / 2, rect.top)
    } else {
        window.prompt(`VIDEO_RECORD:${rect.left + rect.width / 2}x${rect.top}`)
    }
}

function audioRecord() {
    var rect = audioRecordButton.getBoundingClientRect()
    if (use_qt) {
        window.jsbridge.openAudioRecorder(rect.left + rect.width / 2, rect.top)
    } else {
        window.prompt(`AUDIO_RECORD:${rect.left + rect.width / 2}x${rect.top}`)
    }
}

/**
 * Clear all messages.
 */
/* exported clearMessages */
function clearMessages() {
    historyBufferIndex = 0
    canLazyLoad = false
    while (messages.firstChild) {
        messages.removeChild(messages.firstChild)
    }

    if (use_qt) {
        backToBottomBtn.style.visibility="hidden"
        window.jsbridge.emitMessagesCleared()
    } else {
        window.prompt("MESSAGE_CLEARED")
    }
}

/**
 * Convert text to HTML.
 */
function escapeHtml(html) {
    var text = document.createTextNode(html)
    var div = document.createElement("div")
    div.appendChild(text)
    return div.innerHTML
}

/**
 * Get the youtube video id from a URL.
 * @param url
 */
function youtube_id(url) {
    const regExp = /^.*(youtu\.be\/|v\/|\/u\/w|embed\/|watch\?v=|&v=)([^#&?]*).*/
    const match = url.match(regExp)
    return (match && match[2].length == 11) ? match[2] : null
}

/**
 * Emojis in a <pre> without <b> can disappears in a gtk webkit container
 * So for now, use this workaround.
 */
function replace_emojis(text) {
    let emojis_regex = /(?:[\u2700-\u27bf]|(?:\ud83c[\udde6-\uddff]){2}|[\ud800-\udbff][\udc00-\udfff])[\ufe0e\ufe0f]?(?:[\u0300-\u036f\ufe20-\ufe23\u20d0-\u20f0]|\ud83c[\udffb-\udfff])?(?:\u200d(?:[^\ud800-\udfff]|(?:\ud83c[\udde6-\uddff]){2}|[\ud800-\udbff][\udc00-\udfff])[\ufe0e\ufe0f]?(?:[\u0300-\u036f\ufe20-\ufe23\u20d0-\u20f0]|\ud83c[\udffb-\udfff])?)*/g

    text = text.replace(emojis_regex, (emoji) => "</pre><b>" + emoji + "</b><pre>")
    return text
}

/**
 * Returns HTML message from the message text, cleaned and linkified.
 * @param message_text
 */
function getMessageHtml(message_text) {
    const escaped_message = escapeHtml(message_text)

    var linkified_message = linkifyHtml(escaped_message, linkifyOptions)  // eslint-disable-line no-undef
    const textPart = document.createElement("pre")
    if (use_qt) {
        textPart.innerHTML = linkified_message
    } else {
        textPart.innerHTML = replace_emojis(linkified_message)
    }
    var hyperlinks = textPart.getElementsByTagName("a")
    for (var i = 0; i < hyperlinks.length; i++){
        hyperlinks[i].classList.add("text_link")
    }

    return textPart.outerHTML
}

/**
 * Returns the message status, formatted for display
 * @param message_delivery_status
 */
/* exported getMessageDeliveryStatusText */
function getMessageDeliveryStatusText(message_delivery_status) {
    return use_qt ? i18nStringData[message_delivery_status] :  i18n.gettext(message_delivery_status)
}

/**
 * Returns the message date, formatted for display
 */
function getMessageTimestampText(message_timestamp, custom_format) {
    const date = new Date(1000 * message_timestamp)
    if (custom_format) {
        return formatDate(date)
    } else {
        return date.toLocaleString()
    }
}

/**
 * Update timestamps.
 * @param message_div
 */
function updateTimestamps(messages_div) {
    const timestamps = messages_div.getElementsByClassName("timestamp")
    for (var c = timestamps.length - 1; c >= 0; --c) {
        var timestamp = timestamps.item(c)
        timestamp.innerHTML = getMessageTimestampText(timestamp.getAttribute("message_timestamp"), true)
    }
}

/**
 * Convert a value in filesize
 */
function humanFileSize(bytes) {
    var thresh = 1024
    if (Math.abs(bytes) < thresh) {
        return bytes + " B"
    }
    var units = ["kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"]
    var u = -1
    do {
        bytes /= thresh
        ++u
    } while (Math.abs(bytes) >= thresh && u < units.length - 1)
    return bytes.toFixed(1) + " " + units[u]
}

/**
 * Change the value of the progress bar.
 *
 * @param progress_bar
 * @param message_object
 */
function updateProgressBar(progress_bar, message_object) {
    var delivery_status = message_object["delivery_status"]
    if (delivery_status === "ongoing" && "progress" in message_object && message_object["progress"] !== 100) {
        var progress_percent = (100 * message_object["progress"] / message_object["totalSize"])
        if (progress_percent !== 100)
            progress_bar.childNodes[0].setAttribute("style", "display: block; width: " + progress_percent + "%")
        else
            progress_bar.setAttribute("style", "display: none;")
    } else
        progress_bar.setAttribute("style", "display: none;")
}

/**
 * Check if a status is an error status
 * @param
 */
function isErrorStatus(status) {
    return (status === "failure"
         || status === "awaiting peer timeout"
         || status === "canceled"
         || status === "unjoinable peer")
}

/**
 * Build a new file interaction
 * @param message_id
 */
function fileInteraction(message_id) {
    var message_wrapper = document.createElement("div")
    message_wrapper.setAttribute("class", "message_wrapper")

    var transfer_info_wrapper = document.createElement("div")
    transfer_info_wrapper.setAttribute("class", "transfer_info_wrapper")
    message_wrapper.appendChild(transfer_info_wrapper)

    /* Buttons at the left for status information or accept/refuse actions.
       The text is bold and clickable. */
    var left_buttons = document.createElement("div")
    left_buttons.setAttribute("class", "left_buttons")
    transfer_info_wrapper.appendChild(left_buttons)

    var full_div = document.createElement("div")
    full_div.setAttribute("class", "full")
    full_div.style.visibility = "hidden"
    full_div.style.display = "none"

    var filename_wrapper = document.createElement("div")
    filename_wrapper.setAttribute("class", "truncate-ellipsis")

    var message_text = document.createElement("span")
    message_text.setAttribute("class", "filename")
    filename_wrapper.appendChild(message_text)

    // And information like size or error message.
    var informations_div = document.createElement("div")
    informations_div.setAttribute("class", "informations")

    var text_div = document.createElement("div")
    text_div.setAttribute("class", "text")
    text_div.addEventListener("click", function () {
        // ask ring to open the file
        const filename = document.querySelector("#message_" + message_id + " .full").innerText
        if (use_qt) {
            window.jsbridge.openFile(filename)
        } else {
            window.prompt(`OPEN_FILE:${filename}`)
        }
    })

    text_div.appendChild(filename_wrapper)
    text_div.appendChild(full_div)
    text_div.appendChild(informations_div)
    transfer_info_wrapper.appendChild(text_div)

    // And finally, a progress bar
    var message_transfer_progress_bar = document.createElement("span")
    message_transfer_progress_bar.setAttribute("class", "message_progress_bar")

    var message_transfer_progress_completion = document.createElement("span")
    message_transfer_progress_bar.appendChild(message_transfer_progress_completion)
    message_wrapper.appendChild(message_transfer_progress_bar)
    message_transfer_progress_completion.setAttribute("style", "display: none;")

    const internal_mes_wrapper = document.createElement("div")
    internal_mes_wrapper.setAttribute("class", "internal_mes_wrapper")
    internal_mes_wrapper.appendChild(message_wrapper)

    return internal_mes_wrapper
}

function buildMsgTable(message_direction) {
    var tbl = document.createElement("table")

    var row0 = document.createElement("tr")
    var sender_image_cell = document.createElement("td")
    sender_image_cell.setAttribute("class", "sender_image_cell")
    var msg_cell = document.createElement("td")
    msg_cell.setAttribute("class", "msg_cell")

    row0.appendChild((message_direction === "in") ? sender_image_cell : msg_cell)
    row0.appendChild((message_direction === "in") ? msg_cell : sender_image_cell)

    tbl.appendChild(row0)

    var row1 = document.createElement("tr")
    var dummy_cell = document.createElement("td")
    dummy_cell.setAttribute("class", "dummy_cell")
    var timestamp_cell = document.createElement("td")
    timestamp_cell.setAttribute("class", "timestamp_cell")

    row1.appendChild((message_direction === "in") ? dummy_cell : timestamp_cell)
    row1.appendChild((message_direction === "in") ? timestamp_cell : dummy_cell)

    tbl.appendChild(row1)

    return tbl
}

/**
 * Build information text for passed file interaction message object
 *
 * @param message_object message object containing file interaction info
 */
function buildFileInformationText(message_object) {
    var informations_txt = getMessageTimestampText(message_object["timestamp"], true)
    if (message_object["totalSize"] && message_object["progress"]) {
        if (message_object["delivery_status"] === "finished") {
            informations_txt += " - " + humanFileSize(message_object["totalSize"])
        } else {
            informations_txt += " - " + humanFileSize(message_object["progress"])
                         + " / " + humanFileSize(message_object["totalSize"])
        }
    }

    return informations_txt + " - " + getMessageDeliveryStatusText(message_object["delivery_status"])
}

/**
 * Update a file interaction (icons + filename + status + progress bar)
 *
 * @param message_div the message to update
 * @param message_object new informations
 * @param forceTypeToFile
 */
function updateFileInteraction(message_div, message_object, forceTypeToFile = false) {
    if (!message_div.querySelector(".informations")) return // media

    if (!message_object["text"]) {
        message_div.style.visibility = "collapse"
    } else {
        message_div.style.visibility = "visible"
    }

    var acceptSvg = "<svg height=\"24\" viewBox=\"0 0 24 24\" width=\"24\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M0 0h24v24H0z\" fill=\"none\"/><path d=\"M9 16.2L4.8 12l-1.4 1.4L9 19 21 7l-1.4-1.4L9 16.2z\"/></svg>",
        refuseSvg = "<svg height=\"24\" viewBox=\"0 0 24 24\" width=\"24\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M19 6.41L17.59 5 12 10.59 6.41 5 5 6.41 10.59 12 5 17.59 6.41 19 12 13.41 17.59 19 19 17.59 13.41 12z\"/><path d=\"M0 0h24v24H0z\" fill=\"none\"/></svg>",
        fileSvg = "<svg class=\"filesvg\" height=\"24\" viewBox=\"0 0 24 24\" width=\"24\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M16.5 6v11.5c0 2.21-1.79 4-4 4s-4-1.79-4-4V5c0-1.38 1.12-2.5 2.5-2.5s2.5 1.12 2.5 2.5v10.5c0 .55-.45 1-1 1s-1-.45-1-1V6H10v9.5c0 1.38 1.12 2.5 2.5 2.5s2.5-1.12 2.5-2.5V5c0-2.21-1.79-4-4-4S7 2.79 7 5v12.5c0 3.04 2.46 5.5 5.5 5.5s5.5-2.46 5.5-5.5V6h-1.5z\"/><path d=\"M0 0h24v24H0z\" fill=\"none\"/></svg>",
        warningSvg = "<svg class=\"filesvg\" height=\"24\" viewBox=\"0 0 24 24\" width=\"24\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M0 0h24v24H0z\" fill=\"none\"/><path d=\"M1 21h22L12 2 1 21zm12-3h-2v-2h2v2zm0-4h-2v-4h2v4z\"/></svg>"
    if (is_swarm) {
        acceptSvg = "<svg xmlns=\"http://www.w3.org/2000/svg\" height=\"24px\" viewBox=\"0 0 24 24\" width=\"24px\"><path d=\"M0 0h24v24H0z\" fill=\"none\"/><path d=\"M19 9h-4V3H9v6H5l7 7 7-7zM5 18v2h14v-2H5z\"/></svg>"
    }
    var message_delivery_status = message_object["delivery_status"]
    var message_direction = message_object["direction"]
    var message_id = message_object["id"]
    var message_text = message_object["text"]

    if (isImage(message_text) && message_delivery_status === "finished" && displayLinksEnabled && !forceTypeToFile) {
        // Replace the old wrapper by the downloaded image
        var old_wrapper = message_div.querySelector(".internal_mes_wrapper")
        if (old_wrapper) {
            old_wrapper.parentNode.removeChild(old_wrapper)
        }

        var errorHandler = function () {
            var wrapper = message_div.querySelector(".internal_mes_wrapper")
            var message_wrapper = message_div.querySelector(".message_wrapper")
            if (message_wrapper) {
                message_wrapper.parentNode.removeChild(message_wrapper)
            }

            var media_wrapper = message_div.querySelector(".media_wrapper")
            if (media_wrapper) {
                media_wrapper.parentNode.removeChild(media_wrapper)
            }

            var new_interaction = fileInteraction(message_id)
            var new_message_wrapper = new_interaction.querySelector(".message_wrapper")
            wrapper.prepend(new_message_wrapper)
            updateFileInteraction(message_div, message_object, true)
        }

        var new_wrapper = mediaInteraction(message_id, message_direction, message_text, null, errorHandler)
        message_div.insertBefore(new_wrapper, message_div.querySelector(".menu_interaction"))
        message_div.querySelector("img").id = message_id
        message_div.querySelector("img").msg_obj = message_object
        return
    }

    if (isAudio(message_text) && message_delivery_status === "finished" && displayLinksEnabled && !forceTypeToFile) {
        // Replace the old wrapper by the downloaded audio
        var old_wrapper = message_div.querySelector(".message_wrapper")
        if (old_wrapper) {
            old_wrapper.parentNode.removeChild(old_wrapper)
        }

        var errorHandler = function () {
            var wrapper = message_div.querySelector(".internal_mes_wrapper")
            var wrapper_audio_video = message_div.querySelector(".message_type_audio_video_transfer")
            wrapper_audio_video.parentNode.classList.remove("no-audio-overlay")
            wrapper_audio_video.parentNode.removeChild(wrapper_audio_video)

            var message_wrapper = message_div.querySelector(".message_wrapper")
            if (message_wrapper) {
                message_wrapper.parentNode.removeChild(message_wrapper)
            }

            var media_wrapper = message_div.querySelector(".audio")
            if (media_wrapper) {
                media_wrapper.parentNode.removeChild(media_wrapper)
            }

            var new_interaction = fileInteraction(message_id)
            var new_message_wrapper = new_interaction.querySelector(".message_wrapper")
            wrapper.prepend(new_message_wrapper)
            updateFileInteraction(message_div, message_object, true)
        }

        const new_wrapper = document.createElement("audio")
        new_wrapper.onerror = errorHandler
        new_wrapper.setAttribute("src", "file://" + message_text)
        new_wrapper.setAttribute("controls", "controls")
        var audio_type = "audio/ogg"
        if (message_text.toLowerCase().match(/\.(ogg)$/)) {
            audio_type = "audio/ogg"
        } else if (message_text.toLowerCase().match(/\.(flac)$/)) {
            audio_type = "audio/flac"
        } else if (message_text.toLowerCase().match(/\.(wav)$/)) {
            audio_type = "audio/wav"
        } else if (message_text.toLowerCase().match(/\.(mpeg)$/)) {
            audio_type = "audio/mpeg"
        }
        new_wrapper.setAttribute("type", audio_type)
        new_wrapper.setAttribute("class", "audio")
        const audio_video_wrapper = document.createElement("div")
        audio_video_wrapper.setAttribute("class", "message_type_audio_video_transfer")
        audio_video_wrapper.appendChild(new_wrapper)
        message_div.querySelector(".internal_mes_wrapper").insertBefore(audio_video_wrapper, message_div.querySelector(".timestamp"))
        audio_video_wrapper.parentNode.classList.add("no-audio-overlay")
        return
    }

    if (isVideo(message_text) && message_delivery_status === "finished" && displayLinksEnabled && !forceTypeToFile) {
        // Replace the old wrapper by the downloaded audio
        var old_wrapper = message_div.querySelector(".message_wrapper")
        if (old_wrapper) {
            old_wrapper.parentNode.removeChild(old_wrapper)
        }

        var errorHandler = function () {
            var wrapper = message_div.querySelector(".internal_mes_wrapper")
            var wrapper_audio_video = message_div.querySelector(".message_type_audio_video_transfer")
            wrapper_audio_video.parentNode.classList.remove("no-audio-overlay")
            wrapper_audio_video.parentNode.removeChild(wrapper_audio_video)

            var message_wrapper = message_div.querySelector(".message_wrapper")
            if (message_wrapper) {
                message_wrapper.parentNode.removeChild(message_wrapper)
            }

            var media_wrapper = message_div.querySelector(".video")
            if (media_wrapper) {
                media_wrapper.parentNode.removeChild(media_wrapper)
            }

            var new_interaction = fileInteraction(message_id)
            var new_message_wrapper = new_interaction.querySelector(".message_wrapper")
            wrapper.prepend(new_message_wrapper)
            updateFileInteraction(message_div, message_object, true)
        }

        var hideContext = function () {return false}
        const new_wrapper = document.createElement("video")
        new_wrapper.onerror = errorHandler
        new_wrapper.oncontextmenu = hideContext
        new_wrapper.setAttribute("src", "file://" + message_text)
        new_wrapper.setAttribute("controls", "controls")
        var audio_type = "video/mp4"
        if (message_text.toLowerCase().match(/\.(mp4)$/)) {
            audio_type = "video/mp4"
        } else if (message_text.toLowerCase().match(/\.(avi)$/)) {
            audio_type = "video/avi"
        } else if (message_text.toLowerCase().match(/\.(mov)$/)) {
            audio_type = "video/mov"
        } else if (message_text.toLowerCase().match(/\.(webm)$/)) {
            audio_type = "video/webm"
        } else if (message_text.toLowerCase().match(/\.(rmvb)$/)) {
            audio_type = "video/rmvb"
        }
        new_wrapper.setAttribute("type", audio_type)
        new_wrapper.setAttribute("class", "video")
        const audio_video_wrapper = document.createElement("div")

        audio_video_wrapper.setAttribute("class", "message_type_audio_video_transfer")
        audio_video_wrapper.appendChild(new_wrapper)
        message_div.querySelector(".internal_mes_wrapper").insertBefore(audio_video_wrapper, message_div.querySelector(".timestamp"))
        audio_video_wrapper.parentNode.classList.add("no-video-overlay")
        return
    }

    // Set informations text
    var informations_div = message_div.querySelector(".informations")
    informations_div.innerText = buildFileInformationText(message_object)

    // Update flat buttons
    var left_buttons = message_div.querySelector(".left_buttons")
    left_buttons.innerHTML = ""

    if (message_delivery_status === "awaiting host" ||
        message_delivery_status.indexOf("ongoing") === 0) {

        if (message_delivery_status.indexOf("ongoing") !== 0) {
            // add buttons to accept or refuse a call.
            var accept_button = document.createElement("div")
            accept_button.innerHTML = acceptSvg
            accept_button.setAttribute("title", use_qt ? i18nStringData["Accept"] :
                i18n.gettext("Accept"))
            accept_button.setAttribute("class", "flat-button accept")
            accept_button.onclick = function () {
                if (use_qt) {
                    window.jsbridge.acceptFile(message_id)
                } else {
                    window.prompt(`ACCEPT_FILE:${message_id}`)
                }
            }
            left_buttons.appendChild(accept_button)
        }

        if (!is_swarm || message_delivery_status.indexOf("ongoing") === 0) {
            var refuse_button = document.createElement("div")
            refuse_button.innerHTML = refuseSvg
            refuse_button.setAttribute("title", use_qt ? i18nStringData["Refuse"] :
                i18n.gettext("Refuse"))
            refuse_button.setAttribute("class", "flat-button refuse")
            refuse_button.onclick = function () {
                if (use_qt) {
                    window.jsbridge.refuseFile(message_id)
                } else {
                    window.prompt(`REFUSE_FILE:${message_id}`)
                }
            }
            left_buttons.appendChild(refuse_button)
        }

    } else if (message_delivery_status !== "awaiting peer") {
        var status_button = document.createElement("div")
        var statusFile = fileSvg
        if (isErrorStatus(message_delivery_status))
            statusFile = warningSvg
        status_button.innerHTML = statusFile
        status_button.setAttribute("class", "flat-button")
        left_buttons.appendChild(status_button)
    }

    message_div.querySelector(".full").innerText = message_text
    var displayName = message_object["displayName"]
    if (!displayName || displayName === "") {
        displayName = message_text.split("/").pop() // If non swarm displayName will be not set, we can take the path
    }
    message_div.querySelector(".filename").innerText = displayName
    updateProgressBar(message_div.querySelector(".message_progress_bar"), message_object)

    if (!message_div.querySelector(`.dropdown_${message_id}`)) {
        if (!is_swarm || message_delivery_status === "finished") {
            var message_dropdown = buildMessageDropdown(message_id, message_object["displayName"])
            message_div.appendChild(message_dropdown)
        }
    }
}

/**
 * Return if a file is an image
 * @param file
 */
function isImage(file) {
    return file.toLowerCase().match(/\.(jpeg|jpg|gif|png)$/) !== null
}

/**
 * Return if a file is an audio
 * @param file
 */
function isAudio(file) {
    return file.toLowerCase().match(/\.(mp3|mpeg|ogg|flac|wav)$/) !== null
}

/**
 * Return if a file is an video
 * @param file
 */
function isVideo(file) {
    return file.toLowerCase().match(/\.(mp4|avi|webm|mov|rmvb)$/) !== null
}

/**
 * Return if a file is a youtube video
 * @param file
 */
function isYoutubeVideo(file) {
    const availableProtocols = ["http:", "https:"]
    const videoHostname = ["youtube.com", "www.youtube.com", "youtu.be"]
    const urlParser = document.createElement("a")
    urlParser.href = file
    return (availableProtocols.includes(urlParser.protocol) && videoHostname.includes(urlParser.hostname))
}

/**
 * Build a container for passed video thumbnail
 * @param linkElt video thumbnail div
 */
function buildVideoContainer(linkElt) {
    const containerElt = document.createElement("div")
    containerElt.setAttribute("class", "containerVideo")
    const playDiv = document.createElement("div")
    playDiv.setAttribute("class", "playVideo")
    playDiv.innerHTML = "<svg fill=\"#ffffff\" viewBox=\"0 0 24 24\" xmlns=\"http://www.w3.org/2000/svg\">\
        <path d=\"M8 5v14l11-7z\"/>\
        <path d=\"M0 0h24v24H0z\" fill=\"none\"/>\
    </svg>"
    linkElt.appendChild(playDiv)
    containerElt.appendChild(linkElt)

    return containerElt
}

/**
 * Try to show an image
 * @param message_id
 * @param link to show
 * @param errorHandler the new media's onerror field will be set to this function
 */
function mediaInteraction(message_id, message_direction, link, errorHandler) {
    /* TODO promise?
     Try to display images. */
    const media_wrapper = document.createElement("div")
    media_wrapper.setAttribute("class", "media_wrapper")
    const linkElt = document.createElement("a")
    linkElt.href = link
    linkElt.style.textDecoration = "none"
    linkElt.style.border = "none"
    const imageElt = document.createElement("img")

    imageElt.src = link

    /* Note, here, we don't check the size of the image.
     in the future, we can check the content-type and content-length with a request
     and maybe disable svg */

    if (isInitialLoading) {
        /* During initial load, make sure the scrollbar stays at the bottom.
           Also, the final scrollHeight is only known after the last image was
           loaded. We want to display a specific number of messages screens so
           we have to set up a callback (on_image_load_finished) which will
           check on that and reschedule a new display batch if not enough
           messages have been loaded in the DOM. */
        imagesLoadingCounter++
        imageElt.onload = function () {
            back_to_bottom()
            on_image_load_finished()
        }

        if (errorHandler) {
            imageElt.onerror = function () {
                errorHandler()
                back_to_bottom()
                on_image_load_finished()
            }
        }
    } else if (messages.scrollTop >= messages.scrollHeight - messages.clientHeight - scrollDetectionThresh) {
        /* Keep the scrollbar at the bottom. Images are loaded asynchronously and
           the scrollbar position is changed each time an image is loaded and displayed.
           In order to make sure the scrollbar stays at the bottom, reset scrollbar
           position each time an image was loaded. */
        imageElt.onload = back_to_bottom

        if (errorHandler) {
            imageElt.onerror = function () {
                errorHandler()
                back_to_bottom()
            }
        }
    } else if (errorHandler) {
        imageElt.onerror = errorHandler
    }

    linkElt.appendChild(imageElt)
    media_wrapper.appendChild(linkElt)
    const internal_mes_wrapper = document.createElement("div")
    internal_mes_wrapper.setAttribute("class", "internal_mes_wrapper")
    internal_mes_wrapper.appendChild(media_wrapper)

    return internal_mes_wrapper
}

/**
 * @param container_link the anchor tag that is the parent of the text container
 * @param title title of the website page
 * @param description description of the website
 * @param url url that is sent in the message
 */
function buildPreviewTextContainer(container_link, title, description, url){
    var text_container = document.createElement("div")
    text_container.setAttribute("class", "preview_text_container")
    container_link.appendChild(text_container)

    var card_title = document.createElement("pre")
    card_title.setAttribute("class", "preview_card_title")
    title !== null ? card_title.appendChild(document.createTextNode(title)) : card_title.appendChild(document.createTextNode(""))
    text_container.appendChild(card_title)
    if (description !== null && description !== "" && description !== undefined){
        var card_subtitle = document.createElement("p")
        card_subtitle.setAttribute("class", "preview_card_subtitle")
        card_subtitle.appendChild(document.createTextNode(description))
        text_container.appendChild(card_subtitle)
    }
    // reduce url to just its domain for aesthetic purposes
    var domain = (new URL(url))
    domain = (domain.hostname).replace("www.", "")
    var card_link = document.createElement("p")
    card_link.setAttribute("class", "preview_card_link")
    card_link.appendChild(document.createTextNode(domain))
    text_container.appendChild(card_link)
}

/**
 * @param internal_message_wrapper the message wrapper of the message being manipulated
 * @param title title retrieved from url
 * @param img image retrieved form url
 * @param description description of website
 * @param url url that was sent
 * @param message_direction whether the message is being received or sent
 */
function buildPreviewHtml(internal_message_wrapper, title, img, description, url, message_direction) {
    // Create a wrapper for the preview
    var external_preview_wrapper = document.createElement("div")
    external_preview_wrapper.setAttribute("class", "msg_cell_with_preview")
    // Create the cell that will wrap the preview
    var preview_wrapper = document.createElement("div")
    external_preview_wrapper.appendChild(preview_wrapper)
    var timestamp = internal_message_wrapper.getElementsByClassName("timestamp")[0]
    // check if theres a timesstamp and append accordingly
    timestamp ? internal_message_wrapper.insertBefore(external_preview_wrapper, timestamp) : internal_message_wrapper.appendChild(external_preview_wrapper)
    preview_wrapper.setAttribute("class", message_direction === "in" ? "preview_wrapper_in" : "preview_wrapper_out")
    var container = document.createElement("div")
    container.setAttribute("class", "preview_card_container")
    preview_wrapper.appendChild(container)
    var card_container = document.createElement("div")
    message_direction === "in" ? card_container.setAttribute("class", "card_container_in") : card_container.setAttribute("class", "card_container_out")
    container.appendChild(card_container)
    var container_link = document.createElement("a")
    container_link.setAttribute("class", "preview_container_link")
    // add http to ensure that users will be taken
    // to the webpage when the element is clicked
    // and set url of anchor tag
    if (url !== null){
        if (!url.includes("http://") && !url.includes("https://")){
            url = "http://".concat(url)
        }
        container_link.setAttribute("href", url)
        container_link.setAttribute("target", "_self")
    }
    card_container.appendChild(container_link)
    // create image and set image found as property if one exists
    var card_image = document.createElement("img")
    card_image.setAttribute("class", "preview_image")
    if (img !== null && img !== undefined){
        card_image.setAttribute("src", img)
        card_image.onerror = function(){
            card_image.remove()
            var msg_wrap = internal_message_wrapper.getElementsByClassName("message_wrapper_with_image_preview")
            if (msg_wrap.length > 0){
                msg_wrap[0].classList.add("message_wrapper_without_image_preview")
                msg_wrap[0].classList.remove("message_wrapper_with_image_preview")
            }
        }
    }
    else if (img === null){
        var msg_wrap = internal_message_wrapper.getElementsByClassName("message_wrapper_with_image_preview")[0]
        msg_wrap.classList.remove("message_wrapper_with_image_preview")
        msg_wrap.classList.add("message_wrapper_without_image_preview")
    }
    // if there is an image associated with the hyperlink
    // then the message's css must be adjusted accordingly
    container_link.appendChild(card_image)
    // create container that will hold the title, description and domain
    buildPreviewTextContainer(container_link, title, description, url)
}


/**
 * Retrieves information needed for hyperlink preview and creates html element accordingly
 * @param message_id id of the message that contains specificied url
 * @param message_text text containing hyperlink
 * @param message_direction string determining whether the message had been sent or received
 */
function createHyperlinkPreview(message_id, message_text, message_direction) {
    var title = null
    var description = null
    var img = null
    var link_array = linkify.find(message_text)
    var url = link_array[0].href
    //scrolling
    var atEnd = messages.scrollTop >= messages.scrollHeight - messages.clientHeight - scrollDetectionThresh

    fetch(url).then(function (response) {
        return response.text()
    }).then(function (html) {
        // create DOM from html string
        var parser = new DOMParser()
        var doc = parser.parseFromString(html, "text/html")
        if (!url.includes("twitter.com")){
            title = getTitle(doc)
            img = getImage(doc, url)
            description = getDescription(doc)
        } else{
            title = "Twitter. It's what's happening."
        }
        // Retrieve message that will be manipulated
        var message_string = "message_"
        var message_id_string = message_string.concat(message_id)
        var main_text = document.getElementById(message_id_string)
        // ensure that message exists
        if (main_text === null ){
            return
        }
        // Get message wrapper
        var internal_message_wrapper = main_text.getElementsByClassName("internal_mes_wrapper")[0]
        internal_message_wrapper.getElementsByClassName("message_wrapper")[0].classList.add("message_wrapper_with_image_preview")
        internal_message_wrapper.getElementsByClassName("message_text")[0].classList.add("message_text_with_preview")

        buildPreviewHtml(internal_message_wrapper, title, img, description, url, message_direction)
        // fix scrolling
        if (atEnd) {
            messages.scrollTop = messages.scrollHeight
        }
    }).catch(function (err) {
        // Error occured while fetching document
        console.warn("Warning", err)
    })
}

/**
 * Build a new hyperlink interaction
 * @param message_id id of the message
 * @param message_direction string determining whether the message had been sent or received
 * @param the text itself of the message
 * @param htmlText the DOM to show
 */
function hyperlinkInteraction(message_id, message_direction, message_text, htmlText) {
    var main_text = textInteraction(message_id, message_direction, htmlText)
    createHyperlinkPreview (message_id, message_text, message_direction)
    return main_text
}

/**
 * Build a new text interaction
 * @param message_id
 * @param htmlText the DOM to show
 */
function textInteraction(message_id, message_direction, htmlText) {
    const message_wrapper = document.createElement("div")
    message_wrapper.setAttribute("class", "message_wrapper")
    var message_text = document.createElement("div")
    message_text.setAttribute("class", "message_text")
    message_text.setAttribute("dir", "auto")
    message_text.innerHTML = htmlText
    message_wrapper.appendChild(message_text)
    // TODO STATUS

    const internal_mes_wrapper = document.createElement("div")
    internal_mes_wrapper.setAttribute("class", "internal_mes_wrapper")
    internal_mes_wrapper.appendChild(message_wrapper)


    return internal_mes_wrapper
}

/**
 * Build message dropdown
 * @return a message dropdown for passed message id
 */
function buildMessageDropdown(message_id, display_name) {
    const menu_element = document.createElement("div")
    menu_element.setAttribute("class", "menu_interaction")
    menu_element.innerHTML =
        `<input type="checkbox" id="showmenu${message_id}" class="showmenu">
     <label for="showmenu${message_id}">
       <svg fill="#888888" height="12" viewBox="0 0 24 24" width="12" xmlns="http://www.w3.org/2000/svg">
         <path d="M0 0h24v24H0z" fill="none"/>
         <path d="M6 10c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2zm12 0c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2zm-6 0c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2z"/>
       </svg>
     </label>`
    menu_element.onclick = function () {
        const button = this.querySelector(".showmenu")
        button.checked = !button.checked
    }
    menu_element.onmouseleave = function () {
        const button = this.querySelector(".showmenu")
        button.checked = false
    }
    const dropdown = document.createElement("div")
    const dropdown_classes = [
        "dropdown",
        `dropdown_${message_id}`
    ]
    dropdown.setAttribute("class", dropdown_classes.join(" "))

    const save = document.createElement("div")
    save.setAttribute("class", "menuoption")
    if (is_swarm) {
        if (use_qt) {
            save.innerHTML = "Copy to downloads"
        } else {
            save.innerHTML = i18n.gettext("Copy to downloads")
        }
        save.msg_id = message_id
        save.display_name = display_name
        save.onclick = function () {
            if (use_qt) {
                window.jsbridge.copyToDownloads(this.msg_id, this.display_name)
            } else {
                window.prompt(`COPY:${this.msg_id}:${this.display_name}`)
            }
        }
        dropdown.appendChild(save)
    } else {
        const remove = document.createElement("div")
        remove.setAttribute("class", "menuoption")
        if (use_qt) {
            remove.innerHTML = "Delete"
        } else {
            remove.innerHTML = i18n.gettext("Delete")
        }
        remove.msg_id = message_id
        remove.onclick = function () {
            if (use_qt) {
                window.jsbridge.deleteInteraction(`${this.msg_id}`)
            } else {
                window.prompt(`DELETE_INTERACTION:${this.msg_id}`)
            }
        }
        dropdown.appendChild(remove)
    }
    menu_element.appendChild(dropdown)

    return menu_element
}

/**
 * Update a text interaction (text)
 * @param message_div the message to update
 * @param delivery_status the status of the message
 */
function updateTextInteraction(message_div, delivery_status) {
    if (!message_div.querySelector(".message_text")) return // media
    const message_text = message_div.querySelector(".message_text")
    const message_in = message_div.querySelector(".message_in")
    switch(delivery_status)
    {
    case "ongoing":
    case "sending":
        message_text.style.color = "#888"
        break
    case "failure":
        var failure_div = message_div.querySelector(".failure")
        if (!failure_div) {
            failure_div = document.createElement("div")
            failure_div.setAttribute("class", "failure")
            failure_div.innerHTML = "<svg overflow=\"visible\" viewBox=\"0 -2 16 14\" height=\"16px\" width=\"16px\"><path class=\"status-x x-first\" stroke=\"#AA0000\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"3\" fill=\"none\" d=\"M4,4 L12,12\"/><path class=\"status-x x-second\" stroke=\"#AA0000\" stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"3\" fill=\"none\" d=\"M12,4 L4,12\"/></svg>"
            // add failure animation to message
            message_div.insertBefore(failure_div, message_div.querySelector(".menu_interaction"))
        }
        if (message_in)
            message_text.style.color = "var(--message-in-txt)"
        else
            message_text.style.color = "var(--message-out-txt)"
        break
    case "sent":
    case "finished":
    case "unknown":
    case "read":
        if (message_in)
            message_text.style.color = "var(--message-in-txt)"
        else
            message_text.style.color = "var(--message-out-txt)"
        break
    default:
        break
    }
}

/**
 * Build a new interaction (call or contact)
 */
function actionInteraction() {
    var message_wrapper = document.createElement("div")
    message_wrapper.setAttribute("class", "message_wrapper")

    // A file interaction contains buttons at the left of the interaction
    // for the status or accept/refuse buttons
    var left_buttons = document.createElement("div")
    left_buttons.setAttribute("class", "left_buttons")
    message_wrapper.appendChild(left_buttons)

    // Also contains a bold clickable text
    var text_div = document.createElement("div")
    text_div.setAttribute("class", "text")
    message_wrapper.appendChild(text_div)
    return message_wrapper
}

/**
 * Update a call interaction (icon + text)
 * @param message_div the message to update
 * @param message_object new informations
 */
function updateCallInteraction(message_div, message_object) {
    const outgoingCall = "<svg fill=\"#219d55\" height=\"24\" viewBox=\"0 0 24 24\" width=\"24\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M0 0h24v24H0z\" fill=\"none\"/><path d=\"M9 5v2h6.59L4 18.59 5.41 20 17 8.41V15h2V5z\"/></svg>"
    const callMissed = "<svg fill=\"#dc2719\" height=\"24\" viewBox=\"0 0 24 24\" width=\"24\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M0 0h24v24H0z\" fill=\"none\"/><path d=\"M19.59 7L12 14.59 6.41 9H11V7H3v8h2v-4.59l7 7 9-9z\"/></svg>"
    const outgoingMissed = "<svg fill=\"#dc2719\" height=\"24\" viewBox=\"0 0 24 24\" width=\"24\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\"><defs><path d=\"M24 24H0V0h24v24z\" id=\"a\"/></defs><clipPath id=\"b\"><use overflow=\"visible\" xlink:href=\"#a\"/></clipPath><path clip-path=\"url(#b)\" d=\"M3 8.41l9 9 7-7V15h2V7h-8v2h4.59L12 14.59 4.41 7 3 8.41z\"/></svg>"
    const callReceived = "<svg fill=\"#219d55\" height=\"24\" viewBox=\"0 0 24 24\" width=\"24\" xmlns=\"http://www.w3.org/2000/svg\"><path d=\"M0 0h24v24H0z\" fill=\"none\"/><path d=\"M20 5.41L18.59 4 7 15.59V9H5v10h10v-2H8.41z\"/></svg>"

    const message_text = message_object["text"]
    const message_direction = message_object["direction"]
    const message_duration = message_object["duration"]
    const message_sender = message_object["sender"]

    const missed = message_duration === 0 ? true : false

    message_div.querySelector(".text").innerText = message_text

    var left_buttons = message_div.querySelector(".left_buttons")
    left_buttons.innerHTML = ""
    var status_button = document.createElement("div")
    var statusFile = ""
    if (missed)
        statusFile = (message_direction === "in") ? callMissed : outgoingMissed
    else
        statusFile = (message_direction === "in") ? callReceived : outgoingCall
    status_button.innerHTML = statusFile
    status_button.setAttribute("class", "flat-button")
    left_buttons.appendChild(status_button)
}

/**
 * Update a contact interaction (icon + text)
 * @param message_div the message to update
 * @param message_object new informations
 */
function updateContactInteraction(message_div, message_object) {
    const message_text = message_object["text"]

    message_div.querySelector(".text").innerText = message_text
}

/**
 * Remove an interaction from the conversation
 * @param interaction_id
 */
/* exported removeInteraction */
function removeInteraction(interaction_id) {
    var interaction = document.getElementById(`message_${interaction_id}`)
    if (!interaction) {
        return
    }

    if (interaction.previousSibling) {
        /* if element was the most recently received message, make sure the
           last_message property is given away to the previous sibling */
        if (interaction.classList.contains("last_message")) {
            interaction.previousSibling.classList.add("last_message")
        }

        /* same for timestamp */
        var timestamp = interaction.querySelector(".timestamp")
        var previousTimeStamp = interaction.previousSibling.querySelector(".timestamp")
        if (timestamp && !previousTimeStamp) {
            interaction.previousSibling.querySelector(".internal_mes_wrapper").appendChild(timestamp)
        }
    }

    var firstMessage = getPreviousInteraction(interaction)
    var secondMessage = getNextInteraction(interaction)

    updateSequencing(firstMessage, secondMessage)

    interaction.parentNode.removeChild(interaction)
}

/**
 * Build a message div for passed message object
 * @param message_object to treat
 */
function buildNewMessage(message_object) {
    const message_id = message_object["id"]
    const message_type = message_object["type"]
    const message_text = message_object["text"]
    const message_direction = message_object["direction"]
    const delivery_status = message_object["delivery_status"]
    const message_sender_contact_method = message_object["sender_contact_method"]

    var classes = [
        "message",
        `message_${message_direction}`,
        `message_type_${message_type}`
    ]

    var type = ""
    var message_div = document.createElement("div")
    message_div.setAttribute("id", `message_${message_id}`)
    message_div.setAttribute("class", classes.join(" "))

    const need_sender = (message_type === "data_transfer" || message_type === "text")
    if (need_sender) {
        var message_sender_image = document.createElement("span")
        message_sender_image.setAttribute("class", `sender_image sender_image_${message_sender_contact_method}`)
        message_div.appendChild(message_sender_image)
    }

    // Build main content
    if (message_type === "data_transfer") {
        if (isImage(message_text) && delivery_status === "finished" && displayLinksEnabled) {
            var errorHandler = function () {
                var wrapper = message_div.querySelector(".internal_mes_wrapper")
                var message_wrapper = message_div.querySelector(".message_wrapper")
                if (message_wrapper) {
                    message_wrapper.parentNode.removeChild(message_wrapper)
                }

                var media_wrapper = message_div.querySelector(".media_wrapper")
                if (media_wrapper) {
                    media_wrapper.parentNode.removeChild(media_wrapper)
                }

                var new_interaction = fileInteraction(message_id)
                var new_message_wrapper = new_interaction.querySelector(".message_wrapper")
                wrapper.prepend(new_message_wrapper)
                updateFileInteraction(message_div, message_object, true)
            }
            message_div.append(mediaInteraction(message_id, message_direction, message_text, null, errorHandler))
            message_div.querySelector("img").id = message_id
            message_div.querySelector("img").msg_obj = message_object
        } else {
            message_div.append(fileInteraction(message_id))
            updateProgressBar(message_div.querySelector(".message_progress_bar"), message_object)
        }
        if (!message_div.querySelector(`.dropdown_${message_id}`)) {
            if (is_swarm && delivery_status === "finished") {
                var message_dropdown = buildMessageDropdown(message_id, message_object["displayName"])
                message_div.appendChild(message_dropdown)
            }
        }
    } else if (message_type === "text") {
        // TODO add the possibility to update messages (remove and rebuild)
        const htmlText = getMessageHtml(message_text)
        if (displayLinksEnabled) {
            const parser = new DOMParser()
            const DOMMsg = parser.parseFromString(htmlText, "text/xml")
            const links = DOMMsg.querySelectorAll("a")
            if (DOMMsg.childNodes.length && links.length) {
                var isTextToShow = (DOMMsg.childNodes[0].childNodes.length > 1)
                if (!isTextToShow && (isImage(message_text))) {
                    type = "media"
                    message_div.append(mediaInteraction(message_id, message_direction, message_text))
                }
            }
        }
        if (type !== "media" && linkify.find(message_text).length > 0 && displayLinksEnabled){
            message_div.append(hyperlinkInteraction(message_id, message_direction, message_text, htmlText))
        }
        else if (type !== "media") {
            type = "text"
            message_div.append(textInteraction(message_id, message_direction, htmlText))
        }
    } else if (message_type === "call" || message_type === "contact") {
        message_div.append(actionInteraction())
    } else {
        const temp = document.createElement("div")
        temp.innerText = message_type
        message_div.appendChild(temp)
    }

    if (message_type !== "typing" && !is_swarm && !message_div.querySelector(`.dropdown_${message_id}`)) {
        var message_dropdown = buildMessageDropdown(message_id)
        message_div.appendChild(message_dropdown)
    }

    return message_div
}

function addSenderImage(message_div, message_type, message_sender_contact_method) {
    const need_sender = (message_type === "data_transfer" || message_type === "text")
    if (need_sender) {
        var sender_image_cell = message_div.querySelector(".sender_image_cell")
        if (sender_image_cell) {
            var message_sender_image = document.createElement("span")
            var cssSafeStr = message_sender_contact_method.replace(/@/g, "_").replace(/\./g, "_")
            message_sender_image.setAttribute("class", `sender_image sender_image_${cssSafeStr}`)
            sender_image_cell.appendChild(message_sender_image)
        } else {
            console.warn("can't find sender_image_cell")
        }
    }
}

/**
 * Build a timestamp for passed message object
 * @param message_object to treat
 */
function buildNewTimestamp(message_object) {
    const message_type = message_object["type"]
    const message_direction = message_object["direction"]
    const message_timestamp = message_object["timestamp"]

    const formattedTimestamp = getMessageTimestampText(message_timestamp, true)
    const date_elt = document.createElement("div")

    date_elt.innerText = formattedTimestamp
    var typeIsCallOrContact = (message_type === "call" || message_type === "contact")
    var timestamp_div_classes = ["timestamp", typeIsCallOrContact ? "timestamp_action" : `timestamp_${message_direction}`]
    date_elt.setAttribute("class", timestamp_div_classes.join(" "))
    date_elt.setAttribute("message_timestamp", message_timestamp)

    return date_elt
}

/**
 * Add a message to the conversation.
 * @param message_object to treat
 * @param new_message if it's a new message or if we need to update
 * @param insert_after if we want the message at the end or the top of the conversation
 * @param messages_div
 */
function addOrUpdateMessage(message_object, new_message, insert_after = true, messages_div) {
    const message_id = message_object["id"]
    const message_type = message_object["type"]
    const message_direction = message_object["direction"]
    const delivery_status = message_object["delivery_status"]

    var message_div = messages_div.querySelector("#message_" + message_id)
    if (new_message) {
        if (message_div)
            return;
        message_div = buildNewMessage(message_object)

        /* Show timestamp if either:
           - message has type call or contact
           - or most recently added timestamp in this set is different
           - or message is the first message in this set */

        var date_elt = buildNewTimestamp(message_object)
        var timestamp = messages_div.querySelector(".timestamp")

        if (message_type === "call" || message_type === "contact") {
            message_div.querySelector(".message_wrapper").appendChild(date_elt)
        } else if (insert_after || !timestamp || timestamp.className !== date_elt.className
                                || timestamp.innerHTML !== date_elt.innerHTML) {
            message_div.querySelector(".internal_mes_wrapper").appendChild(date_elt)
        }

        var isGenerated = message_type === "call" || message_type === "contact"
        if (isGenerated) {
            message_div.classList.add("generated_message")
        }

        if (insert_after) {
            var previousMessage = messages_div.lastChild
            messages_div.appendChild(message_div)
            computeSequencing(previousMessage, message_div, null, insert_after)
            if (previousMessage) {
                if (previousMessage.id === "message_typing") {
                    previousMessage.parentNode.removeChild(previousMessage)
                    message_div.parentNode.appendChild(previousMessage)
                } else {
                    previousMessage.classList.remove("last_message")
                    message_div.classList.add("last_message")
                }
            }

            /* When inserting at the bottom we should also check that the
               previously sent message does not have the same timestamp.
               If it's the case, remove it.*/
            if (message_div.previousSibling) {
                var previous_timestamp = message_div.previousSibling.querySelector(".timestamp")
                if (previous_timestamp &&
                    previous_timestamp.className === date_elt.className &&
                    previous_timestamp.innerHTML === date_elt.innerHTML &&
                    !message_div.previousSibling.classList.contains("last_of_sequence")) {
                    previous_timestamp.parentNode.removeChild(previous_timestamp)
                }
            }
        } else {
            var nextMessage = messages_div.firstChild
            messages_div.prepend(message_div)
            computeSequencing(message_div, nextMessage, null, insert_after)
        }
    }

    if (!is_swarm && isErrorStatus(delivery_status) && message_direction === "out") {
        const dropdown = messages_div.querySelector(`.dropdown_${message_id}`)
        if (!dropdown.querySelector(".retry")) {
            const retry = document.createElement("div")
            retry.setAttribute("class", "retry")
            if (use_qt) {
                retry.innerHTML = "Retry"
            } else {
                retry.innerHTML = i18n.gettext("Retry")
            }
            retry.msg_id = message_id
            retry.onclick = function () {
                if (use_qt) {
                    window.jsbridge.retryInteraction(`${this.msg_id}`)
                } else {
                    window.prompt(`RETRY_INTERACTION:${this.msg_id}`)
                }
            }
            dropdown.insertBefore(retry, message_div.querySelector(".delete"))
        }
    }

    // Update informations if needed
    if (message_type === "data_transfer")
        updateFileInteraction(message_div, message_object)
    if (message_type === "text" && message_direction === "out")
    // Modify sent status if necessary
        updateTextInteraction(message_div, delivery_status)
    if (message_type === "call")
        updateCallInteraction(message_div, message_object)
    if (message_type === "contact")
        updateContactInteraction(message_div, message_object)

    // Clean timestamps
    updateTimestamps(messages_div)
}

function getNextInteraction(interaction, includeLazyLoadedBlock = true) {
    var nextInteraction = interaction.nextSibling
    if (!nextInteraction && includeLazyLoadedBlock) {
        var nextBlock = interaction.parentNode.nextElementSibling
        if (nextBlock) {
            nextInteraction = nextBlock.firstElementChild
        }
    }
    return nextInteraction
}

function getPreviousInteraction(interaction, includeLazyLoadedBlock = true) {
    var previousInteraction = interaction.previousSibling
    if (!previousInteraction && includeLazyLoadedBlock) {
        var previousBlock = interaction.parentNode.previousElementSibling
        if (previousBlock) {
            previousInteraction = previousBlock.lastElementChild
        }
    }
    return previousInteraction
}

function isSequenceBreak(firstMessage, secondMessage, insert_after = true) {
    if (!firstMessage || !secondMessage) {
        return false
    }
    var first_message_direction = firstMessage.classList.contains("message_out") ? "out" : "in"
    var second_message_direction = secondMessage.classList.contains("message_out") ? "out" : "in"
    if (second_message_direction != first_message_direction) {
        return true
    }
    var firstMessageIsGenerated = firstMessage.classList.contains("generated_message")
    var secondMessageIsGenerated = secondMessage.classList.contains("generated_message")
    if (firstMessageIsGenerated != secondMessageIsGenerated) {
        return true
    }
    if (insert_after) {
        const internal_message_wrapper = firstMessage.querySelector(".internal_mes_wrapper")
        if (internal_message_wrapper) {
            const firstTimestamp = internal_message_wrapper.querySelector(".timestamp")
            return !!(firstTimestamp) && firstTimestamp.innerHTML !== "just now"
        }
        return false
    } else {
        const internal_message_wrapper = firstMessage.querySelector(".internal_mes_wrapper")
        if (internal_message_wrapper) {
            return !!(internal_message_wrapper.querySelector(".timestamp"))
        }
        return false
    }
}

function updateSequencing(firstMessage, secondMessage) {
    if (firstMessage) {
        if (secondMessage) {
            var sequence_break = isSequenceBreak(firstMessage, secondMessage, false)
            if (sequence_break) {
                if (firstMessage.classList.contains("middle_of_sequence")) {
                    firstMessage.classList.remove("middle_of_sequence")
                    firstMessage.classList.add("last_of_sequence")
                } else if (firstMessage.classList.contains("first_of_sequence")) {
                    firstMessage.classList.remove("first_of_sequence")
                    firstMessage.classList.add("single_message")
                }
                if (secondMessage.classList.contains("middle_of_sequence")) {
                    secondMessage.classList.remove("middle_of_sequence")
                    secondMessage.classList.add("first_of_sequence")
                } else if (secondMessage.classList.contains("last_of_sequence")) {
                    secondMessage.classList.remove("last_of_sequence")
                    secondMessage.classList.add("single_message")
                }
            } else {
                if (firstMessage.classList.contains("last_of_sequence")) {
                    firstMessage.classList.remove("last_of_sequence")
                    firstMessage.classList.add("middle_of_sequence")
                } else if (firstMessage.classList.contains("single_message")) {
                    firstMessage.classList.remove("single_message")
                    firstMessage.classList.add("first_of_sequence")
                }
                if (secondMessage.classList.contains("first_of_sequence")) {
                    secondMessage.classList.remove("first_of_sequence")
                    secondMessage.classList.add("middle_of_sequence")
                } else if (secondMessage.classList.contains("single_message")) {
                    secondMessage.classList.remove("single_message")
                    secondMessage.classList.add("last_of_sequence")
                }
            }
        } else {
            // this is the last interaction of the conversation
            if (firstMessage.classList.contains("first_of_sequence")) {
                firstMessage.classList.remove("first_of_sequence")
                firstMessage.classList.add("single_message")
            } else if (firstMessage.classList.contains("middle_of_sequence")) {
                firstMessage.classList.remove("middle_of_sequence")
                firstMessage.classList.add("last_of_sequence")
            }
        }
    } else if (secondMessage) {
        // this is the first interaction of the conversation
        if (secondMessage.classList.contains("middle_of_sequence")) {
            secondMessage.classList.remove("middle_of_sequence")
            secondMessage.classList.add("first_of_sequence")
        } else if (secondMessage.classList.contains("last_of_sequence")) {
            secondMessage.classList.remove("last_of_sequence")
            secondMessage.classList.add("single_message")
        }
    }
}

function computeSequencing(firstMessage, secondMessage, lazyLoadingBlock, insert_after = true) {
    if (insert_after) {
        if (secondMessage) {
            var secondMessageIsGenerated = secondMessage.classList.contains("generated_message")
            if (firstMessage && !secondMessageIsGenerated) {
                var firstMessageIsGenerated = firstMessage.classList.contains("generated_message")
                var sequence_break = isSequenceBreak(firstMessage, secondMessage)
                if (sequence_break) {
                    secondMessage.classList.add("single_message")
                } else {
                    if (firstMessage.classList.contains("single_message")) {
                        firstMessage.classList.remove("single_message")
                        firstMessage.classList.add("first_of_sequence")
                    } else if (firstMessage.classList.contains("last_of_sequence")) {
                        firstMessage.classList.remove("last_of_sequence")
                        firstMessage.classList.add("middle_of_sequence")
                    }
                    if (firstMessageIsGenerated) {
                        secondMessage.classList.add("single_message")
                    } else {
                        secondMessage.classList.add("last_of_sequence")
                    }
                }
            } else if (!secondMessageIsGenerated) {
                secondMessage.classList.add("single_message")
            }
        }
    } else if (firstMessage) {
        var firstMessageIsGenerated = firstMessage.classList.contains("generated_message")
        if (secondMessage && !firstMessageIsGenerated) {
            var secondMessageIsGenerated = secondMessage.classList.contains("generated_message")
            var sequence_break = isSequenceBreak(firstMessage, secondMessage, false)
            if (sequence_break) {
                firstMessage.classList.add("single_message")
            } else {
                if (secondMessage.classList.contains("single_message")) {
                    secondMessage.classList.remove("single_message")
                    secondMessage.classList.add("last_of_sequence")
                } else if (secondMessage.classList.contains("first_of_sequence")) {
                    secondMessage.classList.remove("first_of_sequence")
                    secondMessage.classList.add("middle_of_sequence")
                }
                if (secondMessageIsGenerated) {
                    firstMessage.classList.add("single_message")
                } else {
                    firstMessage.classList.add("first_of_sequence")
                }
            }
        } else if (!firstMessageIsGenerated) {
            firstMessage.classList.add("single_message")
        }
    }
}

/**
 * Wrapper for addOrUpdateMessage.
 *
 * Add or update a message and make sure the scrollbar position
 * is refreshed correctly
 *
 * @param message_object message to be added
 */
/* exported addMessage */
function addMessage(message_object) {
    if (!messages.lastChild) {
        var block_wrapper = document.createElement("div")
        messages.append(block_wrapper)
    }

    exec_keeping_scroll_position(addOrUpdateMessage, [message_object, true, undefined, messages.lastChild])
}

/**
 * Update a message that was previously added with addMessage and
 * make sure the scrollbar position is refreshed correctly
 *
 * @param message_object message to be updated
 */
/* exported updateMessage */
function updateMessage(message_object) {
    var message_div = messages.querySelector("#message_" + message_object["id"])
    exec_keeping_scroll_position(addOrUpdateMessage, [message_object, false, undefined, message_div.parentNode])
}

/**
 * Called whenever an image has finished loading. Check lazy loading status
 * once all images have finished loading.
 */
function on_image_load_finished() {
    imagesLoadingCounter--

    if (!imagesLoadingCounter) {
        /* This code is executed once all images have been loaded. */
        check_lazy_loading()
    }
}

/**
 * Make sure at least initialScrollBufferFactor screens of messages are
 * available in the DOM.
 */
function check_lazy_loading() {
    if (!canLazyLoad)
        return
    if (messages.scrollHeight < initialScrollBufferFactor * messages.clientHeight
        && historyBufferIndex !== historyBuffer.length) {
        /* Not enough messages loaded, print a new batch. Enable isInitialLoading
           as reloading a single batch might not be sufficient to fulfill our
           criteria (we want to be called back again to check on that) */
        isInitialLoading = true
        printHistoryPart(messages, 0)
        isInitialLoading = false
    }
}

/**
 * Display 'scrollBuffer' messages from history in passed div (reverse order).
 *
 * @param messages_div that should be modified
 * @param fixedAt maintain scrollbar at the specified position
 */
function printHistoryPart(messages_div, fixedAt, allLoaded=true) {
    if (historyBufferIndex === historyBuffer.length) {
        return
    }
    /* If first element is a spinner, remove it */
    if (messages_div.firstChild && messages_div.firstChild.id === "lazyloading-icon") {
        messages_div.removeChild(messages_div.firstChild)
    }

    /* Elements are appended to a wrapper div. This div has no style
       properties, it allows us to add all messages at once to the main
       messages div. */
    var block_wrapper = document.createElement("div")

    messages_div.prepend(block_wrapper)

    for (var i = 0; i < scrollBuffer && historyBufferIndex < historyBuffer.length; ++historyBufferIndex && ++i) {
        // TODO on-screen messages should be removed from the buffer
        addOrUpdateMessage(historyBuffer[historyBuffer.length - 1 - historyBufferIndex], true, false, block_wrapper)
    }

    var lastMessage = block_wrapper.lastChild

    updateSequencing(lastMessage, getNextInteraction(lastMessage))

    var absoluteLastMessage = messages_div.lastChild.lastChild
    if (absoluteLastMessage) {
        absoluteLastMessage.classList.add("last_message")
    }

    /* Add ellipsis (...) at the top if there are still messages to load */
    if (!allLoaded) {
        var llicon = document.createElement("span")
        llicon.id = "lazyloading-icon"
        llicon.innerHTML = "<svg xmlns=\"http://www.w3.org/2000/svg\" fill=\"#888888\" width=\"24\" height=\"24\" viewBox=\"0 0 24 24\"><path d=\"M0 0h24v24H0z\" fill=\"none\"/><path d=\"M6 10c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2zm12 0c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2zm-6 0c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2z\"/></svg>"
        messages_div.prepend(llicon)
    }

    if (fixedAt !== undefined) {
        /* update scrollbar position to take text-message -related
           scrollHeight changes in account (not necessary to wait
           for DOM redisplay in this case). Changes due to image
           messages are handled in their onLoad callbacks. */
        back_to_scroll(fixedAt)
        /* schedule a scrollbar position update for changes which
           are neither handled by the previous call nor by onLoad
           callbacks. This call is necessary but not sufficient,
           dropping the previous call would result in visual
           glitches during initial load. */
        setTimeout(function () {back_to_scroll(fixedAt)}, 0)
    }

    if (!imagesLoadingCounter) {
        setTimeout(check_lazy_loading, 0)
    }
}

function hideMessagesDiv()
{
    if (!messages.classList.contains("fade")) {
        messages.classList.add("fade")
    }
}

function showMessagesDiv()
{
    if (messages.classList.contains("fade")) {
        messages.classList.remove("fade")
    }
}

/**
 * Set history buffer, initialize messages div and display a first batch
 * of messages.
 *
 * Make sure that enough messages are displayed to fill initialScrollBufferFactor
 * screens of messages (if enough messages are present in the conversation)
 *
 * @param messages_array should contain history to be printed
 */
/* exported printHistory */
function printHistory(messages_array)
{
    historyBuffer = messages_array

    historyBufferIndex = 0
    isInitialLoading = true
    printHistoryPart(messages, 0)
    isInitialLoading = false

    canLazyLoad = true
    if (use_qt) {
        window.jsbridge.emitMessagesLoaded()
    } else {
        window.prompt("MESSAGES_LOADED")
    }
}


/**
 * Updates history buffer, used for lazy loading messages.
 * Note: for swarm chat is used instead of printHistory, so
 * reset of historyBufferIndex has been moved to clearMessages()
 *
 * @param messages_array should contain history to be printed
 */
/* exported printHistory */
function updateHistory(messages_array, all_loaded)
{
    historyBuffer = messages_array

    printHistoryPart(messages,  messages.scrollHeight, all_loaded)
    canLazyLoad = true

    if (messages.scrollTop === 0) {
        if (use_qt) {
            window.jsbridge.loadMessages(scrollBuffer)
        } else {
            window.prompt(`LOAD_MESSAGES:${scrollBuffer}`)
        }
    }

    if (use_qt) {
        window.jsbridge.emitMessagesLoaded()
    } else {
        window.prompt("MESSAGES_LOADED")
    }
}

/**
 * Set the image for a given sender
 * set_sender_image object should contain the following keys:
 * - sender: the name of the sender
 * - sender_image: base64 png encoding of the sender image
 *
 * @param set_sender_image_object sender image object as previously described
 */
/* exported setSenderImage */
function setSenderImage(set_sender_image_object)
{
    if (use_qt) {
        var sender_contact_method = set_sender_image_object["sender_contact_method"].replace(/@/g, "_").replace(/\./g, "_"),
            sender_image = set_sender_image_object["sender_image"],
            sender_image_id = "sender_image_" + sender_contact_method,
            invite_sender_image_id = "invite_sender_image_" + sender_contact_method,
            currentSenderImage = document.getElementById(sender_image_id), // Remove the currently set sender image
            style, invite_style

    } else {
        var sender_contact_method = set_sender_image_object["sender_contact_method"],
            sender_image = set_sender_image_object["sender_image"],
            sender_image_id = "sender_image_" + sender_contact_method,
            invite_sender_image_id = "invite_sender_image_" + sender_contact_method,
            currentSenderImage = document.getElementById(sender_image_id), // Remove the currently set sender image
            style, invite_style
    }

    if (currentSenderImage) {
        currentSenderImage.parentNode.removeChild(currentSenderImage)
    }

    // Create a new style element
    style = document.createElement("style")

    style.type = "text/css"
    style.id = sender_image_id
    style.innerHTML = "." + sender_image_id + " {content: url(data:image/png;base64," + sender_image + ");height: 32px; width: 32px;}"
    document.head.appendChild(style)

    invite_style = document.createElement("style")

    invite_style.type = "text/css"
    invite_style.id = invite_sender_image_id
    invite_style.innerHTML = "." + invite_sender_image_id + " {content: url(data:image/png;base64," + sender_image + ");height: 48px; width: 48px;}"
    document.head.appendChild(invite_style)
}

/**
 * Show Typing indicator
 */
/* exported showTypingIndicator */
function showTypingIndicator(contactUri, isTyping) {
    var message_div
    if (messages.lastChild)
        message_div = messages.lastChild.querySelector("#message_typing")
    else
        return

    if (isTyping === 0) {
        if (message_div) {
            message_div.parentNode.removeChild(message_div)
        }
    } else if (!message_div) {
        message_div = buildNewMessage({
            "id":"typing",
            "type":"text",
            "text":"",
            "direction":"in",
            "delivery_status":"",
            "sender_contact_method": contactUri
        })

        var previousMessage = messages.lastChild.lastChild
        messages.lastChild.appendChild(message_div)
        computeSequencing(previousMessage, message_div, null, true)
        if (previousMessage) {
            previousMessage.classList.remove("last_message")
        }
        message_div.classList.add("last_message")
        let msg_text = message_div.querySelector(".message_text")
        msg_text.innerHTML = " \
                        <div class=\"typing-indicator\"> \
                            <span></span> \
                            <span></span> \
                            <span></span> \
                        </div>"
    }
    updateMesPos()
}

/**
 * Copy Mouse Selected Text and return it
 */
function copy_text_selected() {
    var selObj = document.getSelection()
    var selectedText = selObj.toString()
    return selectedText
}

/**
 * Check if text is selected by mouse
 */
function isTextSelected() {

    var selObj = document.getSelection()
    var selectedText = selObj.toString()

    if (selectedText.length != 0) {
        return true
    }
    return false
}

/**
 * add file (local file) to message area
 */
function addFile_path(path, name, size) {
    var html = "<div class='file_wrapper' data-path='" + path + "'>" +
        "<svg class='svgicon' version='1.1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' x='0px' y='0px' viewBox='0 0 24 24' style='enable-background:new 0 0 24 24;' xml:space='preserve'>" +
        "<path d='M14.2,2.3C14,2.1,13.7,2,13.4,2H5.5C4.8,2,4.3,2.5,4.3,3.2v6.3v1.6v9.7c0,0.7,0.5,1.2,1.2,1.2h13.1c0.7,0,1.2-0.5,1.2-1.2 V18V8.4c0-0.3-0.1-0.6-0.3-0.8L14.2,2.3z M18,7.7H14c0,0,0,0,0,0V3.7L18,7.7z M18.6,20.9C18.6,20.9,18.6,20.9,18.6,20.9l-13.1,0 c0,0,0,0,0,0v-9.7V9.5V3.2c0,0,0,0,0,0h7.4v4.5c0,0.7,0.5,1.2,1.2,1.2h4.5v9.2V20.9z'/>" +
        "</svg>" +
        "<div class='fileinfo'>" +
        "<p>" + name + "</p>" +
        "<p>" + size + "</p>" +
        "</div >" +
        "<button class='btn' onclick='remove(this)'>X</button>" +
        "</div >"
    // At first, visiblity can empty
    if (sendContainer.style.display.length == 0 || sendContainer.style.display == "none") {
        grow_send_container()
        sendContainer.style.display = "flex"
    }
    //add html here since display is set to flex, image will change accordingly
    sendContainer.innerHTML += html
    updateMesPos()
}

/**
 * add image (base64 array) to message area
 */
function addImage_base64(base64) {

    var html = "<div class='img_wrapper'>" +
        "<img src='data:image/png;base64," + base64 + "'/>" +
        "<button class='btn' onclick='remove(this)'>X</button>" +
        "</div >"
    // At first, visiblity can empty
    if (sendContainer.style.display.length == 0 || sendContainer.style.display == "none") {
        grow_send_container()
        sendContainer.style.display = "flex"
    }
    //add html here since display is set to flex, image will change accordingly
    sendContainer.innerHTML += html
    updateMesPos()
}

/**
 * add image (image path) to message area
 */
function addImage_path(path) {

    var html = "<div class='img_wrapper'>" +
        "<img src='" + path +"'/>" +
        "<button class='btn' onclick='remove(this)'>X</button>" +
        "</div >"
    // At first, visiblity can empty
    if (sendContainer.style.display.length == 0 || sendContainer.style.display == "none") {
        grow_send_container()
        sendContainer.style.display = "flex"
    }
    //add html here since display is set to flex, image will change accordingly
    sendContainer.innerHTML += html
    updateMesPos()
}

/**
 * This function adjusts the body paddings so that that the data_transfer_send_container doesn't
 * overlap messages when it grows.
 */
/* exported grow_send_container */
function grow_send_container() {
    exec_keeping_scroll_position(function () {
        backToBottomBtnContainer.style.bottom = "calc(var(--messagebar-size) + 168px)"
    }, [])
    checkSendButton()
}

/**
 * This function adjusts the body paddings so that that the data_transfer_send_container will hide
 * and recover padding bottom
 */
/* exported grow_send_container */
function reduce_send_container() {
    exec_keeping_scroll_position(function () {
        sendContainer.innerHTML = ""
        sendContainer.style.display = "none"
        backToBottomBtnContainer.style.bottom = "var(--messagebar-size)"
        //6em
    }, [])
    checkSendButton()
}

// This function update the bottom of messages window whenever the send_interface changes size when the scroll is at the end
function updateMesPos() {
    if (messages.scrollTop >= messages.scrollHeight - messages.clientHeight - scrollDetectionThresh) {
        back_to_bottom()
    }
    checkSendButton()
}

// Remove current cancel button division  and hide the sendContainer
function remove(e) {
    e.parentNode.parentNode.removeChild(e.parentNode)
    if (sendContainer.innerHTML.length == 0) {
        reduce_send_container()
    }
}

// It's called in qt qwebengine
function pasteKeyDetected(e) {
    e.preventDefault()
    window.jsbridge.emitPasteKeyDetected()
}

// Set the curser to a target position
function setCaretPosition(elem, caretPos) {
    var range

    if (elem.createTextRange) {
        range = elem.createTextRange()
        range.move("character", caretPos)
        range.select()
    } else {
        elem.focus()
        if (elem.selectionStart !== undefined) {
            elem.setSelectionRange(caretPos, caretPos)
        }
    }
}

function replaceText() {
    navigator.clipboard.readText()
        .then(text => {
            var input = messageBarInput
            var currentContent = input.value
            var start = input.selectionStart
            var end = input.selectionEnd
            var output = [currentContent.slice(0, start), text, currentContent.slice(end)].join("")
            input.value = output
            setCaretPosition(input, start + text.length)
            grow_text_area()
        })
        .catch(err => {
            console.error("Failed to read clipboard contents: ", err)
        })
}

/**
 * Change theme
 * @param theme  The dark theme
 */
function setTheme(theme) {
    let root = document.documentElement
    root.setAttribute("style", "\
        --jami-light-blue: rgba(59, 193, 211, 0.3);\
        --jami-dark-blue: #003b4e;\
        --jami-green: #1ed0a8;\
        --jami-green-hover: #1f8b4c;\
        --jami-red: #dc2719;\
        --jami-red-hover: #b02e2c;\
        --text-color: black;\
        --timestamp-color: #333;\
        --message-out-bg: #cfd8dc;\
        --message-out-txt: black;\
        --message-in-bg: #fdfdfd;\
        --message-in-txt: black;\
        --file-in-timestamp-color: #555;\
        --file-out-timestamp-color: #555;\
        --bg-color: #f2f2f2;\
        --navbar-height: 40px;\
        --navbar-padding-top: 8px;\
        --navbar-padding-bottom: var(--navbar-padding-top);\
        --textarea-max-height: 150px;\
        --placeholder-text-color: #d3d3d3;\
        --action-icon-color: var(--jami-dark-blue);\
        --deactivated-icon-color: #BEBEBE;\
        --action-icon-hover-color: var(--jami-light-blue);\
        --action-critical-icon-hover-color: rgba(211, 77, 59, 0.3);\
        --action-critical-icon-press-color: rgba(211, 77, 59, 0.5);\
        --action-critical-icon-color: #4E1300;\
        --action-icon-press-color: rgba(59, 193, 211, 0.5);\
        --invite-hover-color: white;\
        --bg-text-input: white;\
    ")
    if (theme != "") {
        root.setAttribute("style", theme)
    }
}

/**
 * Get the content of the send message text field as a string.
 * This should be called and the client should wait for
 * saveSendMessageContent before calling clearSendMessageContent
 * and printHistory when changing conversations.
 *
 */
/* exported requestSendMessageContent */
function requestSendMessageContent() {
    if (use_qt) {
        window.jsbridge.saveSendMessageContent(messageBarInput.value)
    } else {
        window.prompt("SAVE:" + messageBarInput.value)
    }
}

/**
 * Sets the content of the send message text field.
 * Use an empty string to clear.
 *
 * * @param contentStr  the content
 */
/* exported setSendMessageContent */
function setSendMessageContent(contentStr) {
    messageBarInput.value = contentStr
    // Do not grow if there are no messages loaded
    if (historyBuffer.length !== 0)
        grow_text_area()
    reduce_send_container()
}

function checkSendButton() {
    sendButton.style.display = (messageBarInput.value.length > 0
                                   || sendContainer.innerHTML.length > 0)
        ? "block" : "none"
}
