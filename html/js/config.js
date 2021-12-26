function loadConfiguration() {

    ajaxJSON({
        url: "/rest/config",
        success: function(o) {
            // wifi
            elemSetValue("config_ssid", o.wifi.ssid);
            elemSetValue("config_psk", o.wifi.psk);
            elemSetValue("config_dhcp", (o.wifi.dhcp === true) ? "1" : "0");
            elemSetValue("config_ip", o.wifi.ip);
            elemSetValue("config_gw", o.wifi.gw);
            elemSetValue("config_netmask", o.wifi.netmask);
            elemSetValue("config_dns", o.wifi.dns);

            // ntp
            elemSetValue("config_ntpserver", o.ntp.server);
            elemSetValue("config_timezone", o.ntp.timezone);

            // brewfather
            elemSetValue("config_deviceid", o.brewfather.deviceid);
            elemSetValue("config_streamurl", o.brewfather.logurl);
            elemSetValue("config_apiuser", o.brewfather.apiuser);
            elemSetValue("config_apikey", o.brewfather.apikey);

            // mqtt
            elemSetValue("config_mqttserver", o.mqtt.server);
            elemSetValue("config_mqttport", o.mqtt.port);

            change_dhcp();
        }
    });

}

function postConfiguration() {
    var o = {
        wifi: {
            ssid: elemGetValue("config_ssid"),
            psk: elemGetValue("config_psk"),
            dhcp: (elemGetValue("config_dhcp") === "1"),
            ip: elemGetValue("config_ip"),
            gw: elemGetValue("config_gw"),
            netmask: elemGetValue("config_netmask"),
            dns: elemGetValue("config_dns"),    
        },
        ntp: {
            server: elemGetValue("config_ntpserver"),
            timezone: elemGetValue("config_timezone"),
        },
        brewfather: {
            deviceid: elemGetValue("config_deviceid"),
            logurl: elemGetValue("config_streamurl"),
            apiuser: elemGetValue("config_apiuser"),
            apikey: elemGetValue("config_apikey")
        },
        mqtt: {
            server: elemGetValue("config_mqttserver"),
            port: elemGetValue("config_mqttport")
        }
    };

    var e = $("buttonSaveIcon");
    e.className = e.className.replace("fa-cloud-upload-alt", "fa-spinner fa-spin");

    var b = $("buttonSave");

    ajaxJSON({
        url: "/rest/setconfig",
        method: "POST",
        data: o,
        success: function() {
            setTimeout(function() {
                e.className = e.className.replace("fa-spinner fa-spin", "fa-check-circle");
                b.className = b.className.replace("normal", "success");

                setTimeout(function() {
                    e.className = e.className.replace("fa-check-circle", "fa-cloud-upload-alt");
                    b.className = b.className.replace("success", "normal");
                }, 2000);
            }, 1000);
        },
        error: function(status) {
            setTimeout(function() {
                e.className = e.className.replace("fa-spinner fa-spin", "fa-times-circle");
                b.className = b.className.replace("normal", "failed");

                setTimeout(function() {
                    e.className = e.className.replace("fa-times-circle", "fa-cloud-upload-alt");
                    b.className = b.className.replace("failed", "normal");
                }, 2000);
            }, 1000);
        }
    });
}

function confirmConfiguration() {
    openWindow({
        id: "confirmation",
        title: "Confirmation Required",
        content: "Are you sure you want to overwrite the configuration of the device ?",
        textButton1: "Cancel",
        textButton2: "Store Configuration",
        callbackButton1: function() {
            closeWindow("confirmation");
        },
        callbackButton2: function() {
            closeWindow("confirmation");
            postConfiguration();
        }
    });

}

function selectNetwork() {
    var encTypeIcon = {
        2: "fa-lock",
        4: "fa-lock",
        5: "fa-lock-open",
        7: "fa-unlock",
        8: "fa-lock"
    };

    clearTable("networktable");
    openModal("networklist");

    addRow("networktable", "<i class=\"fas fa-spinner fa-2x fa-spin busy\"></i>", null, null, "nohover");

    ajaxJSON({
        url: "/rest/listnetworks",
        success: function(data) {
            clearTable("networktable");

            var list = data.list;

            list.sort(function(a, b) {
                return a.rssi < b.rssi;
            });

            list.forEach(function(item) {
                var se = "<i class=\"fas " + encTypeIcon[item.enctype] + " fa-fw\">&nbsp;</i><span>" + item.ssid + "</span>";
                addRow("networktable", se, item.rssi + "dBm / Channel " + item.channel, item.ssid);
            });
        },
        error: function(status) {
        }
    });
}

function change_dhcp() {
    var dhcp = (elemGetValue("config_dhcp") === "1");
    var list = document.getElementsByClassName("ipdata");

    for(var i = 0; i < list.length; i++) {
        list[i].style.display = (dhcp ? "none" : "");
    }
}
