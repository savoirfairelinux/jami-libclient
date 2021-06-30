"use strict"

var emojiPicker = undefined

new QWebChannel(qt.webChannelTransport, function (channel) {
    window.jsbridge = channel.objects.jsbridge
})

/* exported init_emoji_picker */
function init_emoji_picker(dark) {
    emojiPicker = new EmojiButton({
        theme: dark ? "dark" : "light"
    })
    emojiPicker.on("emoji", selection => {
        window.jsbridge.emojiIsPicked(selection.emoji)
    })
}

/* exported prepare_to_show */
function prepare_to_show(dark) {
    emojiPicker.setTheme(dark ? "dark" : "light")

    if (emojiPicker.hideInProgress) {
        setTimeout(() => prepare_to_show(), 100)
        return
    }

    emojiPicker.pickerVisible = true
    emojiPicker.wrapper.style.display = 'block'

    emojiPicker.focusTrap.activate()

    emojiPicker.wrapper.style.height = '100%'
    emojiPicker.wrapper.style.width = '100%'

    setTimeout(() => {
        emojiPicker.addEventListeners()
        emojiPicker.setInitialFocus()
    })

    emojiPicker.emojiArea.reset()
}

/* exported prepare_to_hide */
function prepare_to_hide() {
    emojiPicker.hidePicker()
}
