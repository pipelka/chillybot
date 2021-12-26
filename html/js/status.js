function loadStatusInformation(callback) {

    function translateUptime(uptime) {
        var days = uptime / (1000 * 60 * 60 * 24);
        var hours = uptime / (1000 * 60 * 60);
        var min = Math.floor(uptime / (1000 * 60));

        if(isNaN(hours)) {
            return "";
        }

        if(hours < 1.0) {
            return min + " min";
        }

        if(hours < 24.0) {
            var dh = Math.floor(hours);
            return dh + (dh === 1 ? " hr " : " hrs ") + translateUptime((min - 60 * dh) * 60 * 1000);
        }

        var dd = Math.floor(days);
        return dd + (dd === 1 ? " day " : " days ") + translateUptime((hours - 24 * dd) * 60 * 60 * 1000);
    }

    ajaxJSON({
        url: "/rest/status",
        success: function(o) {
            /*elemSetText("version", o.version);
            elemSetText("status_stationid", o.stationid);
            elemSetText("status_chipid", o.chipid);
            elemSetText("status_cpuclock", o.cpu + " Mhz");
            elemSetText("status_uptime", translateUptime(o.uptime));
            elemSetText("status_time", o.timestamp);
            elemSetText("status_freeheap", ((o.heap * 100) / 40960).toFixed(1) + "%");
            elemSetText("status_ssid", o.ssid);
            elemSetText("status_channel", (o.channel == 0) ? "Automatic" : o.channel);
            elemSetText("status_rssi", o.rssi + " dBm");
            elemSetText("status_hostname", o.hostname);
            elemSetText("status_ip", o.ip);
            elemSetText("status_gw", o.gateway);
            elemSetText("status_dns", o.dns);
            elemSetText("status_mac", o.mac);*/

            if(!o.batch || !o.batch.number || !o.batch.name) {
                $("batch").style.display = "none";
            }
            else {
                $("batch").style.display = "block";
                elemSetText("batchno", o.batch.number);
                elemSetText("batchname", o.batch.name);    
            }

            elemSetText("current_temperature", o.temp.current.toFixed(1))
            elemSetText("target_temperature", o.temp.target.toFixed(1));

            if(!window.override || window.override.enabled != o.override.enabled) {
                elemSetValue("override_target", o.override.target);
                elemSetCheck("override", o.override.enabled)
            }
            window.override = o.override;

            if(callback) {
                callback(o);
            }
        },
        error: function(e) {
            if(callback) {
                callback(e);
            }
        }
    });
}

function loadStatusInformationPeriodically() {
    loadStatusInformation(function() {
        setTimeout(loadStatusInformationPeriodically, 2000);
    });

}
