function apply_override(event) {
    const o = {
        enabled: elemGetCheck("override"),
        target: elemGetValue("override_target")
    };

    ajaxJSON({
        url: "/rest/setoverride",
        method: "POST",
        data: o,
        success: function() {
        }
    });
}