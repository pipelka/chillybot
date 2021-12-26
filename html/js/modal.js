function openModal(id) {
    var e = $(id);

    if(!e) {
        return;
    }

    e.style.display = "block";
}

function closeModal(id) {
    var e = $(id);

    if(!e) {
        return;
    }

    e.style.display = "none";
}

function onRowClick(e) {
    if(!e || e === "") {
        return;
    }

    elemSetValue("config_ssid", e);
    closeModal("networklist");
}

function openWindow(options) {
    var e = $(options.id);

    if(!e) {
        return;
    }

    e.className = "modal";
    e.style.display = "block";
    e.innerHTML = "";

    var icon = "<i class='fas fa-question-circle fa-fw fa-3x'></i>";
    var id1 =  options.id + "_1";
    var id2 =  options.id + "_2";
    var style = (options.width ? " style='width: " + options.width + ";'" : "");

    var btn1 = options.textButton1 ? "<button id='" +  id1 + "'>" + options.textButton1 + "</button>" : "";
    var btn2 = options.textButton2 ? "<button id='" +  id2 + "'>" + options.textButton2 + "</button>" : "";

    var html = "<div class='window'" + style + "><div class='title'>" + options.title + "</div>" + icon + "<div class='content'>" + options.content + "</div><div class='dialog-button-bar'>" + btn1 + btn2 + "</div></div>";
    e.insertAdjacentHTML('beforeend', html);

    $(id1).addEventListener("click", options.callbackButton1);
    $(id2).addEventListener("click", options.callbackButton2);
}

function closeWindow(id) {
    var e = $(id);

    if(!e) {
        return;
    }

    e.style.display = "none";
    e.innerHTML = "";
}

function addRow(id, content, morecontent, rowid, classname)
{

    var table = $(id);
    if(!table) {
        return;
    }

    var tabBody = table.getElementsByTagName("tbody").item(0);

    var row = "<tr " + (classname ? "class=\"" + classname + "\"" : "");
    if(rowid) {
        row  += " onclick='onRowClick(\"" + rowid + "\")'";
    }
    row += "><th>" + content + "</th>";
    if(morecontent && morecontent !== "") {
        row += "<td>" + morecontent + "</td>";
    }
    row += "</tr>";

    tabBody.insertAdjacentHTML('beforeend', row);
}

function clearTable(id) {
    var table = $(id);
    if(!table) {
        return;
    }

    var tabBody = table.getElementsByTagName("tbody").item(0);

    tabBody.innerHTML = "";
}
