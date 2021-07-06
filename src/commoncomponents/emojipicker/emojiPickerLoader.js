"use strict"

var emojiPicker = undefined

const HIDE_VARIANT_POPUP = 'hideVariantPopup'
const PICKER_HIDDEN = 'hidden'

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
        setTimeout(() => prepare_to_show(dark), 100)
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
    emojiPicker.hideInProgress = true
    emojiPicker.focusTrap.deactivate()
    emojiPicker.pickerVisible = false

    if (emojiPicker.overlay) {
        document.body.removeChild(emojiPicker.overlay)
        emojiPicker.overlay = undefined
    }

    // In some browsers, the delayed hide was triggering the scroll event handler
    // and stealing the focus. Remove the scroll listener before doing the delayed hide.
    emojiPicker.emojiArea.emojis.removeEventListener(
        'scroll',
        emojiPicker.emojiArea.highlightCategory
    )

    emojiPicker.pickerEl.classList.add('hiding')

    // Let the transition finish before actually hiding the picker so that
    // the user sees the hide animation.
    setTimeout(
        () => {
            emojiPicker.wrapper.style.display = 'none'
            emojiPicker.pickerEl.classList.remove('hiding')

            if (emojiPicker.pickerContent.firstChild !== emojiPicker.emojiArea.container) {
                empty(emojiPicker.pickerContent)
                emojiPicker.pickerContent.appendChild(emojiPicker.emojiArea.container)
            }

            if (emojiPicker.search) {
                emojiPicker.search.clear()
            }

            emojiPicker.events.emit(HIDE_VARIANT_POPUP)

            emojiPicker.hideInProgress = false
            emojiPicker.popper && emojiPicker.popper.destroy()

            emojiPicker.publicEvents.emit(PICKER_HIDDEN)

            window.jsbridge.emojiPickerHideFinished()
        },
        emojiPicker.options.showAnimation ? 170 : 0
    )

    setTimeout(() => {
        document.removeEventListener('click', emojiPicker.onDocumentClick)
        document.removeEventListener('keydown', emojiPicker.onDocumentKeydown)
    })
}
