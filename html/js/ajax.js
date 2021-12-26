function ajax(o) {
    o.method = o.method ? o.method : "GET";

    var xmlhttp = new XMLHttpRequest();

    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState === XMLHttpRequest.DONE) {   // XMLHttpRequest.DONE == 4
            if (xmlhttp.status === 200) {
                if(o.success) {
                    o.success(xmlhttp.responseText);
                }
            }
            else {
                if(o.error) {
                    o.error(xmlhttp.status);
                }
            }
        }
    };

    xmlhttp.open(o.method, o.url, true);

    if(o.data && o.method === "POST") {
        xmlhttp.setRequestHeader("Content-Type", o.contentType);
        xmlhttp.send(o.data);
        return;
    }

    xmlhttp.send();
}

function ajaxJSON(o) {
    ajax({
        url: o.url,
        method: o.method,
        data: JSON.stringify(o.data),
        contentType: "application/json",
        success: function(text) {
            var json = null;
            if(text) {
                json = JSON.parse(text);
            }
            o.success(json);
        },
        error: o.error
    });
}
