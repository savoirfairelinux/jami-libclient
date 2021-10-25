_ = new QWebChannel(qt.webChannelTransport, function (channel) {
    window.jsbridge = channel.objects.jsbridge
})

function log(msg) {
    window.jsbridge.log(msg)
}

function getPreviewInfo(messageId, url) {
    var title = null
    var description = null
    var image = null
    if (!url.includes("http://") && !url.includes("https://")) {
        url = "http://".concat(url)
    }
    fetch(url, {
              mode: 'no-cors',
              headers: {'Set-Cookie': 'SameSite=None; Secure'}
          }).then(function (response) {
        return response.text()
    }).then(function (html) {
        // create DOM from html string
        var parser = new DOMParser()
        var doc = parser.parseFromString(html, "text/html")
        if (!url.includes("twitter.com")){
            title = getTitle(doc)
            image = getImage(doc, url)
            description = getDescription(doc)
            var domain = (new URL(url))
            domain = (domain.hostname).replace("www.", "")
        } else {
            title = "Twitter. It's what's happening."
        }

        window.jsbridge.infoReady(messageId, {
                                      'title': title,
                                      'image': image,
                                      'description': description,
                                      'url': url,
                                      'domain': domain,
                                  })
    }).catch(function (err) {
        log("Error occured while fetching document: " + err)
    })
}

function parseMessage(messageId, message, showPreview) {
    var links = linkify.find(message)
    if (links.length === 0) {
        return
    }
    if (showPreview)
        getPreviewInfo(messageId, links[0].href)
    window.jsbridge.linkifyReady(messageId, linkifyStr(message))
}
