const SCAN_URL = "wifiscan.cgi";
const CHANGE_STA_URL = "changestation.cgi";
const CHANGE_AP_URL = "changeap.cgi";
const REBOOT_URL = "/flash/reboot";

function buildUrl(url, parameters) {
    let qs = "";
    for (const key in parameters) {
        if (parameters.hasOwnProperty(key)) {
            const value = parameters[key];
            qs +=
                encodeURIComponent(key) + "=" + encodeURIComponent(value) + "&";
        }
    }
    if (qs.length > 0) {
        qs = qs.substring(0, qs.length - 1); //chop off last "&"
        url = url + "?" + qs;
    }

    return url;
}

function hideText(name) {
    document.getElementById(name).hidden = true;
}

function showText(name, hideAfter=true) {
    var errNode = document.getElementById(name)
    if (errNode.hidden) {
        document.getElementById(name).hidden = false;
        if(hideAfter){
            setTimeout(hideText, 5000, name)
        }
    }
}

function showTextOnSuccess(successID){
    return function(response){
        if(response.status==200){
            showText(successID)
        }else{
            console.log("failed");
        }
    }
}

function getSTA_SSID() {
    const ssid = document.getElementById("sta_ssid").value
    const ssid_man = document.getElementById("sta_ssid_man").value
    if(ssid.length && ssid != "other"){
        return ssid;
    }
    return ssid_man;
}

function submitSTA() {
    const ssid = getSTA_SSID();
    const pass = document.getElementById("sta_pass").value
    var error = false;
    if (ssid.length == 0) {
        showText("sta_ssid_error")
        error = true
    } else {
        hideText("sta_ssid_error")
    }
    if (pass.length < 8 && pass.length != 0) {
        showText("sta_pass_error")
        error = true
    } else {
        hideText("sta_pass_error")
    }
    if (!error) {
        console.log("fetch: ", buildUrl(CHANGE_STA_URL, {ssid, pass}))
        fetch(buildUrl(CHANGE_STA_URL, {ssid, pass}))
            .then(showTextOnSuccess("sta_success"))
    }
}

function submitAP() {
    const ssid = document.getElementById("ap_ssid").value
    const pass = document.getElementById("ap_pass").value
    const hidden = document.getElementById("ap_hidden").checked
    var error = false;
    if (ssid.length == 0) {
        showText("ap_ssid_error")
        error = true
    } else {
        hideText("ap_ssid_error")
    }
    if (pass.length < 8) {
        showText("ap_pass_error")
        error = true
    } else {
        hideText("ap_pass_error")
    }
    if (!error) {
        console.log("fetch: ", buildUrl(CHANGE_AP_URL, {ssid, pass, hidden}))
        fetch(buildUrl(CHANGE_AP_URL, {ssid, pass, hidden}))
            .then(showTextOnSuccess("ap_success"))
    }
}

function reboot() {
    console.log("fetch: ", REBOOT_URL)
    fetch(REBOOT_URL).then(showTextOnSuccess("reboot_success"))
}

function selectionChanged() {
    const newVal = document.getElementById("sta_ssid").value
    var sta_ssid_div = document.getElementById("sta_ssid_man_div")
    sta_ssid_div.hidden = !(newVal == "other");
    console.log(newVal)
}

function appendToSelect(ssid) {
    var selectNode = document.getElementById("sta_ssid")
    var newNode = document.createElement("OPTION");
    var text = document.createTextNode(ssid);
    selectNode.appendChild(newNode)
    newNode.appendChild(text)
    newNode.value = ssid
}

var ssids = [];
function updateSSIDs() {
    console.log("fetching ssids")
    showText("sta_load", false)
    fetch(SCAN_URL).then(
        (response) => {
            if (response.status !== 200) {
                console.log("failed to fetch");
            } else {
                response.json().then(function (data) {
                    if (data.result.APs == null || data.result.APs.length == 0) {
                        console.log("no APs. should rescan");
                        setTimeout(updateSSIDs, 500) //rescanning
                    }
                    else {
                        delete data.result.APs[0] //discard first (invalid) AP
                        //sort by signal strength
                        data.result.APs.sort((l, r) => parseInt(r.rssi) - parseInt(l.rssi))
                        console.log(data.result.APs);
                        //add to select
                        data.result.APs.forEach((AP) => {
                            var ssid = AP.essid.substring(0, 32)
                            //dont add copies
                            if (ssids.includes(ssid)) { return }
                            ssids.push(ssid);
                            appendToSelect(ssid)
                        }
                        )
                        hideText("sta_load");
                        setTimeout(updateSSIDs, 5000) //rescanning after some time
                    }
                }).catch(function(e){
                    console.log("fail")
                    console.log(e)
                    setTimeout(updateSSIDs, 500)
                });
            }
        }
    ).catch(function(e){
        console.log("fail")
        console.log(e)
        setTimeout(updateSSIDs, 500)
    });
}
updateSSIDs()