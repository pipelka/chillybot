function $(id) {
    return document.getElementById(id);
}

function elemSetText(id, text) {
    $(id).innerHTML = text;
}

function elemSetValue(id, text) {
    if(text && text !== "") {
        $(id).value = text;
    }
}

function elemClearValue(id) {
    $(id).value = "";
}

function elemGetValue(id) {
    return $(id).value;
}

function elemSetCheck(id, checked) {
    $(id).checked = checked;
}

function elemGetCheck(id) {
    return $(id).checked;
}
