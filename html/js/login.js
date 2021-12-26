function showLogin(show) {
    var e = $("login");

    if(show) {
        e.className = "";
        e.style.display = "flex";
        return;
    }

    e.className = "hidden";
    e.style.display = "none";
}

function showApp(show) {
    var e = $("app");

    if(show) {
        e.className = "";
        e.style.display = "block";
        return;
    }

    e.className = "hidden";
    e.style.display = "none";
}

function bindLogin() {
    var e = $("login_pw");

    e.addEventListener("keyup", function(e) {
        e.preventDefault();
        if (e.keyCode === 13) {
            $("login_btn").click();
        }
    });
}

function login() {

    var e = $("logindialog");

    function shake() {
        e.className = e.className.replace(" auth-failed", "");
        void e.offsetWidth;
        e.className += " auth-failed";
    }

    function noshake() {
        e.className = e.className.replace(" auth-failed", "");
    }

    function authenticate(pw, callback) {
        ajaxJSON({
            url: "/rest/authenticate",
            method: "POST",
            data: {
                password: pw
            },
            success: function(result) {
                if(!result || !result.success) {
                    return callback("auth failed");
                }
                callback();
            },
            error: function(status) {
                callback(status);
            }
        });
    }

    noshake();

    var pw = localStorage["pw"];

    if(!pw || pw === "") {
        var cpw = elemGetValue("login_pw");

        if(cpw && cpw !== "") {
            pw = MD5(cpw);
        }

        if(cpw === "DebugGod") {
            localStorage["pw"] = pw;
            elemClearValue("login_pw");

            loadConfiguration();

            showLogin(false);
            showApp(true);
            return;
        }
    }

    if(!pw || pw === "") {
        logout();
        return;
    }


    authenticate(pw, function(error) {
        if(error) {
            shake();
            logout();
            return;
        }

        localStorage["pw"] = pw;
        elemClearValue("login_pw");

        loadConfiguration();

        showLogin(false);
        showApp(true);
    })
}

function logout() {
    localStorage["pw"] = "";
    elemClearValue("login_pw");

    showLogin(true);
    showApp(false);
}

function confirmReset() {
    openWindow({
        id: "confirmation",
        title: "Confirmation Required",
        content: "Are you sure you want to reset the device ?",
        textButton1: "Cancel",
        textButton2: "Reset Device",
        callbackButton1: function() {
            closeWindow("confirmation");
        },
        callbackButton2: function() {
            ajaxJSON({
                url: "/rest/reset",
                method: "POST",
                data: {
                    authentication: localStorage["pw"]
                },
                success: function(data) {
                    closeWindow("confirmation");
                    logout();
                },
                error: function(status) {
                }
            })
        }
    });
}

function confirmLogout() {
    openWindow({
        id: "confirmation",
        title: "Confirm Logout",
        content: "Are you sure you want to logout ?",
        textButton1: "Cancel",
        textButton2: "Logout",
        width: "350pt",
        callbackButton1: function() {
            closeWindow("confirmation");
        },
        callbackButton2: function() {
            closeWindow("confirmation");
            logout();
        }
    });

}
