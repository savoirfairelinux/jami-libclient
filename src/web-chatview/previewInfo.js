/* MIT License

Copyright (c) 2019 Andrej Gajdos

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

/**
 * Retrieves the title of a webpage which is used to fill out the preview of a hyperlink
 * @param doc the DOM of the url that is being previewed
 * @returns the title of the given webpage
 */

function getTitle(doc){
    const og_title = doc.querySelector("meta[property=\"og:title\"]")
    if (og_title !== null && og_title.content.length > 0) {
        return og_title.content
    }
    const twitter_title = doc.querySelector("meta[name=\"twitter:title\"]")
    if (twitter_title !== null && twitter_title.content.length > 0) {
        return twitter_title.content
    }
    const doc_title = doc.title
    if (doc_title !== null && doc_title.length > 0) {
        return doc_title
    }
    if (doc.querySelector("h1") !== null){
        const header_1 = doc.querySelector("h1").innerHTML
        if (header_1 !== null && header_1.length > 0) {
            return header_1
        }
    }
    if (doc.querySelector("h2") !== null){
        const header_2 = doc.querySelector("h2").innerHTML
        if (header_2 !== null && header_2.length > 0) {
            return header_2
        }
    }
    return null
}

/**
 * Obtains a description of the webpage for the hyperlink preview
 * @param doc the DOM of the url that is being previewed
 * @returns a description of the webpage
 */
function getDescription(doc){
    const og_description = doc.querySelector("meta[property=\"og:description\"]")
    if (og_description !== null && og_description.content.length > 0) {
        return og_description.content
    }
    const twitter_description = doc.querySelector("meta[name=\"twitter:description\"]")
    if (twitter_description !== null && twitter_description.content.length > 0) {
        return twitter_description.content
    }
    const meta_description = doc.querySelector("meta[name=\"description\"]")
    if (meta_description !== null && meta_description.content.length > 0) {
        return meta_description.content
    }
    var all_paragraphs = doc.querySelectorAll("p")
    let first_visible_paragraph = null
    for (var i = 0; i < all_paragraphs.length; i++) {
        if ( all_paragraphs[i].offsetParent !== null && !all_paragraphs[i].childElementCount != 0 ) {
            first_visible_paragraph = all_paragraphs[i].textContent
            break
        }
    }
    return first_visible_paragraph
}

/**
 * Gets the image that represents a webpage.
 * @param doc the DOM of the url that is being previewed
 * @returns the image representing the url or null if no such image was found
 */
function getImage(doc) {
    const og_image = doc.querySelector("meta[property=\"og:image\"]")
    if (og_image !== null && og_image.content.length > 0){
        return og_image.content
    }
    const image_rel_link = doc.querySelector("link[rel=\"image_src\"]")
    if (image_rel_link !== null && image_rel_link.href.length > 0){
        return image_rel_link.href
    }
    const twitter_img = doc.querySelector("meta[name=\"twitter:image\"]")
    if (twitter_img !== null && twitter_img.content.length > 0) {
        return twitter_img.content
    }

    let imgs = Array.from(doc.getElementsByTagName("img"))
    if (imgs.length > 0) {
        imgs = imgs.filter(img => {
            let add_image = true
            if (img.naturalWidth > img.naturalHeight) {
                if (img.naturalWidth / img.naturalHeight > 3) {
                    add_image = false
                }
            } else {
                if (img.naturalHeight / img.naturalWidth > 3) {
                    add_image = false
                }
            }
            if (img.naturalHeight <= 50 || img.naturalWidth <= 50) {
                add_image = false
            }
            return add_image
        })
    }
    return null
}
