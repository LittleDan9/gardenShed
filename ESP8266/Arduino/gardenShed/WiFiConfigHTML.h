const char WiFiConfigHTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<head>
    <style>
        #iconsLeft .glyphicon-flash, #iconsRight .glyphicon-flash {
            color: yellow;
            -webkit-text-stroke: 1px black;
            text-shadow: 3px 3px 0 #000, -1px -1px 0 #000, 1px -1px 0 #000, -1px 1px 0 #000, 1px 1px 0 #000;
        }

        #iconsLeft .glyphicon-cloud, #iconsRight .glyphicon-cloud {
            color: white;
            -webkit-text-stroke: 1px black;
            text-shadow: 3px 3px 0 #000, -1px -1px 0 #000, 1px -1px 0 #000, -1px 1px 0 #000, 1px 1px 0 #000;
        }
    </style>
    <script src="https://code.jquery.com/jquery-3.1.1.min.js" integrity="sha256-hVVnYaiADRTO2PzUGmuLJr8BLUSjGIZsDYGmIJLv2b8=" crossorigin="anonymous"></script>
    <!-- Latest compiled and minified CSS -->
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css" integrity="sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u" crossorigin="anonymous">
    <!-- Latest compiled and minified JavaScript -->
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js" integrity="sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa" crossorigin="anonymous"></script>
    <script type="text/javascript">
        function showLoading() {
            $('#status').html("");
            $('#errorMessage').html("");
            $('#loadingFooter').addClass('hidden');
            $('#loadingBar').removeClass('hidden');
            $('#processingModal').modal('show');
        }

        function hideLoading() {
            $('#processingModal').modal('hide');
            $('#loadingFooter').removeClass('hidden');
            $('#loadingBar').addClass('hidden');
        }
    </script>
    <meta name="viewport" content="width=device-width, initial-scale=1">
</head>

<body style="margin:20px;">
    <div class="container">
        <div id="processingModal" class="modal fade" tabindex="-1" role="dialog">
            <div class="modal-dialog" role="document">
                <div class="modal-content">
                    <div class="modal-header">
                        <h4 class="modal-title">Processing - Please Wait</h4>
                    </div>
                    <div class="modal-body">
                        <div class="progress" id="loadingBar">
                            <div class="progress-bar progress-bar-striped active" role="progressbar" aria-valuenow="100" aria-valuemin="0" aria-valuemax="100" style="width: 100%">
                                <span class="sr-only">Please Wait - Loading</span>
                            </div>
                        </div>
                        <div id="status"></div>
                        <div id="errorMessage"></div>
                    </div>
                    <div id="loadingFooter" class="modal-footer">
                        <button type="button" class="btn btn-primary" data-dismiss="modal">OK</button>
                    </div>
                </div>
                <!-- /.modal-content -->
            </div>
            <!-- /.modal-dialog -->
        </div>
        <!-- /.modal -->
        <script type="text/javascript">showLoading();</script>
        <!--Header-->
        <div class="panel panel-default">
            <div class="panel-heading text-center jumbotron">
                <div style="display:inline; vertical-align:middle;" class="heading">
                    <div class="row">
                        <div id="iconsLeft" class="col-sm-4 col-md-3">
                            <h1>
                                <span class="glyphicon glyphicon-flash"></span>
                                <span class="glyphicon glyphicon-cloud"></span>
                            </h1>
                        </div>
                        <div id="title" class="col-sm-4 col-md-6">
                            <h1>
                                <span>Device Setup</span>
                            </h1>
                        </div>
                        <div id="iconsRight" class="col-sm-4 col-md-3">
                            <h1>
                                <span class="glyphicon glyphicon-cloud"></span>
                                <span class="glyphicon glyphicon-flash"></span>
                            </h1>
                        </div>
                    </div>
                </div>
            </div>
            <div class="row">
                <div class="col-sm-offset-1 col-xs-10">
                    <ul class="nav nav-tabs">
                        <li role="presentation"><a href="/">Home</a></li>
                        <li role="presentation" class="active"><a href="/wifi">WiFi Settings</a></li>
                        <li role="presentation"><a href="/config">System Configuration</a></li>
                        <li role="presentation"><a href="/conditions">Current Conditions</a></li>
                        <li role="presentation"><a href="/about">About</a></li>
                    </ul>
                </div>
            </div>
            <div class="panel-body content">
                <div class="row">
                    <div class="col-sm-offset-1 col-xs-10">
                        <form id="rootForm" class="form-horizontal">
                            <!--WiFi Settings Panel-->
                            <div class="panel panel-primary">
                                <div class="panel-heading">
                                    <h2>WiFi Settings</h2>
                                </div>
                                <div class="panel-body">
                                    <div class="row">
                                        <div class="col-xs-10 col-xs-offset-1">

                                            <!--Hostname-->
                                            <div class="form-group">
                                                <label for="txtHostname" class="control-label">Host Name</label>
                                                <input type="text" class="form-control" id="txtHostname" placeholder="Hostname" value="GardenShedConditions" maxlength="30">
                                            </div>

                                            <!--SSID Select Box-->
                                            <div class="form-group">
                                                <label for="SSID" class="control-label">SSID&nbsp;<span id="btnInfo" class="hidden form-control-static glyphicon glyphicon-info-sign" style="cursor:pointer; color:#428bca;"></span></label>
                                                <select class="form-control" id="SSID">
                                                    <option value="-1">Select a WiFi Network</option>
                                                </select>
                                            </div>
                                            <!--Encryption-->
                                            <div id="divSecurity">
                                                <div class="form-group">
                                                    <label for="txtPassword" class="control-label">WiFi Password</label>
                                                    <input type="password" class="form-control" id="txtPassword" placeholder="WiFi Password" maxlength="30">
                                                </div>
                                            </div>

                                            <!--Submit Button-->
                                            <div class="form-group">
                                                <button type="button" id="btnSave" class="btn btn-primary" disabled>Save Configuration</button>
                                            </div>
                                            <!--WiFi Settings Panel-->
                                        </div>
                                    </div>
                                </div>
                            </div>
                            <!--End Panel-->
                        </form>
                    </div>
                </div>
            </div>
        </div>
    </div>
</body>
<!--Script-->
</html>
<script type="text/javascript">
    $(function () {
        //Load WiFi SSID Options
        $.ajax({
            url: '/api/getNetworks',
            data: "",
            dataType: 'json',
            success: function (result) {
                getNetworkSettings();
                if (result != null) {
                    for (var key in result.networks) {
                        if (result.networks.hasOwnProperty(key)) {
                            var network = result.networks[key];
                            //console.log(network);
                            var option = new Option(network.SSID, network.SSID)
                            $('#SSID').append(
                                $(option).
                                    data("security", network.security).
                                    data("rssi", network.RSSI).
                                    data("bssid", network.BSSID).
                                    data("channel", network.channel).
                                    data("ishidden", network.isHidden)
                            );
                            if (network.selected) {
                                $(option).prop('selected', true);
                                setupInfo();
                            }
                        }
                    }
                }
                hideLoading();
            },
            error: function () {
                setModalStatus(false);
                setModalError('Failed to retrieve available networks. Refresh page to try again.');                
            }
        });
    });

    function getNetworkSettings(){
        $.ajax({
            url: '/api/getNetworkSettings',
            data: '',
            dataType: 'json',
            success: function (result) {
                //console.log(result);
                $('#txtHostname').val(result.hostname);
                $('#SSID option').filter(function () { return $(this).html() == result.ssid; }).val();
            }
        });
    }

    /*******************************************************************
    /* Modal Window
    /******************************************************************/
    function setModalStatus(success) {
        $('#status').html('<h2>' + (success ? '<span class=\"glyphicon glyphicon-ok-sign\" style=\"color:#5cb85c\"></span> Success!' : '<span class=\"glyphicon glyphicon-exclamation-sign\" style=\"color:#d9534f\"></span> Failed!') + '</h2>');
    }

    function clearModalStatus() {
        setModalStatus("");
    }

    function setModalError(message) {
        $('#errorMessage').html('<h3>' + message + '</h3>');
    }

    function clearModalError() {
        setModalError("");
    }

    //Buton Save Click
    $('#btnSave').on('click', function () {
        showLoading();
        var toSend = {
            Hostname: $('#txtHostname').val(),
            SSID: $('#SSID option:selected').val(),
            Password: $('#txtPassword').val(),
        };
        $.ajax({
            url: '/wifi/save/',
            data: toSend,
            dataType: 'json',
            success: function (data) {
                $('#loadingBar').addClass('hidden');
                setModalStatus(data.success);
                if (!data.success) {
                    setModalError(data.error);
                }
                $('#loadingFooter').removeClass('hidden');
                //console.log(data);
            },
            error: function () {
                $('#loadingBar').addClass('hidden');
                setModalStatus(false);
                setModalError("Failed to submit changes to device.");
                $('#loadingFooter').removeClass('hidden');
            }

        });
    });

    /*******************************************************************
    /* WiFi Info Box
    /******************************************************************/
    //Setup Popover
    $('#btnInfo').popover({
        title: "WiFi Info",
        content: "Loading...",
        html: true
    });

    //Toggle Popover
    $('#btnInfo').on('click', function () {
        $('#btnInfo').popover('show');
    });

    $('#SSID').on('change', function () {
        setupInfo();
    });

    function setupInfo() {
        var $option = $('#SSID option:selected');
        //Don't allow a submit on -1 selected
        if ($option.val() == -1) {
            $('#btnInfo').addClass('hidden');
            $('#btnSave').prop('disabled', true);
        } else {
            $('#btnInfo').removeClass('hidden');
            $('#btnSave').prop('disabled', false);
        }

        var encType = $option.data('security');
        if (encType == " + String(ENC_TYPE_NONE) + ")
            $('#divSecurity').addClass('hidden');
        else
            $('#divSecurity').removeClass('hidden');

        var wifiInfo = "";
        if ($option.val() != -1) {
            wifiInfo += 'SSID: ' + $option.val() + '<br />';
            wifiInfo += 'Encryption: ' + getEncryptType($option.data('security')) + '<br />';
            wifiInfo += 'RSSI: ' + $option.data('rssi') + '<br />';
            wifiInfo += 'BSSID: ' + $option.data('bssid') + '<br />';
            wifiInfo += 'Channel: ' + $option.data('channel') + '<br />';
            wifiInfo += 'Hidden: ' + $option.data('ishidden');
        }
        $('#btnInfo').attr('data-content', wifiInfo);
    }

    function getEncryptType(encryptType) {
        switch (encryptType) {
            case "5":
                return 'WEP';
            case "2":
                return 'WPA / PSK';
            case "4":
                return 'WPA2 / PSK';
            case "7":
                return 'Open Network';
            case "8":
                return 'WPA / WPA2 / PSK';
            default:
                return 'Unknown - ' + encryptType;
        }
    }
    /*******************************************************************
    /* End WiFi Info Box
    /******************************************************************/
</script>
)=====";
