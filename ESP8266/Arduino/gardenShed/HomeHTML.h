const char HomeHTML[] PROGMEM = R"=====(
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

        .detailItem {
            font-size:18pt;
            margin: 10px 0px;
        }
    </style>
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js" integrity="sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa" crossorigin="anonymous"></script>
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css" integrity="sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u" crossorigin="anonymous">
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
                        <li role="presentation" class="active"><a href="/">Home</a></li>
                        <li role="presentation"><a href="/wifi">WiFi Settings</a></li>
                        <li role="presentation"><a href="/config">System Configuration</a></li>
                        <li role="presentation"><a href="/conditions">Current Conditions</a></li>
                        <li role="presentation"><a href="/about">About</a></li>
                    </ul>
                </div>
            </div>      
            <div class="panel-body content">
                    <div class="row">
                        <div class="col-sm-offset-1 col-xs-10 detailItem">
                            <div class="panel panel-default">
                                <div class="panel-body">
                                    <div class="row">
                                        <div class="col-sm-offset-1 col-xs-10 detailItem">
                                            <span class="glyphicon glyphicon-tag"></span>&nbsp;Chip ID: <span id="chipId"></span>
                                        </div>
                                    </div>
                                    <div class="row">
                                        <div class="col-sm-offset-1 col-xs-10 detailItem">
                                            <span class="glyphicon glyphicon-user"></span>&nbsp;Device Name: <span id="devName"></span>
                                        </div>
                                    </div>
                                    <div class="row">
                                        <div class="col-sm-offset-1 col-xs-10 detailItem">
                                            <span class="glyphicon glyphicon-map-marker"></span>&nbsp;Device Location: <span id="devLocation"></span>
                                        </div>
                                    </div>    
                                    <hr />                                                                    
                                    <div class="row">
                                        <div class="col-sm-offset-1 col-xs-10 detailItem">                
                                            <span class="glyphicon glyphicon-flash"></span>&nbsp;Supply Voltage: <span id="supplyV"></span>
                                        </div>
                                    </div>                        
                                    <div class="row">
                                        <div class="col-sm-offset-1 col-xs-10 detailItem">                        
                                            <span class="glyphicon glyphicon-off"></span>&nbsp;Last Reset Reason: <span id="resetReason"></span>
                                        </div>
                                    </div>
                                    <hr />                        
                                    <div class="row">
                                        <div class="col-sm-offset-1 col-xs-10 detailItem">                        
                                            <span class="glyphicon glyphicon-compressed"></span>&nbsp;Free Heap: <span id="freeHeap"></span>
                                        </div>
                                    </div>
                                    <hr />                        
                                    <div class="row">
                                        <div class="col-sm-offset-1 col-xs-10 detailItem">                        
                                            <span class="glyphicon glyphicon-tag"></span>&nbsp;Flash Chip ID: <span id="flashId"></span>                        
                                        </div>
                                    </div>                        
                                    <div class="row">
                                        <div class="col-sm-offset-1 col-xs-10 detailItem">                        
                                            <span class="glyphicon glyphicon-floppy-disk"></span>&nbsp;Flash Chip Size: <span id="flashSize"></span>
                                        </div>
                                    </div>                        
                                    <div class="row">
                                        <div class="col-sm-offset-1 col-xs-10 detailItem">                        
                                            <span class="glyphicon glyphicon-floppy-disk"></span>&nbsp;Flash Chip Size (SDK): <span id="flashSizeSDK"></span>
                                        </div>
                                    </div>                        
                                    <div class="row">
                                        <div class="col-sm-offset-1 col-xs-10 detailItem">                        
                                            <span class="glyphicon glyphicon-random"></span>&nbsp;Flash Chip Frequency: <span id="flashFreq"></span>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>                        
                    </div>
                </div>
            </div>
        </div>
    </div>
</body>
<!--Script-->
<script type="text/javascript">
    $(function () {
        //Load WiFi SSID Options
        $.ajax({
            url: '/api/getDeviceDetails',
            data: "",
            dataType: 'json',
            success: function (result) {
                if (result != null) {
                    $('#chipId').html(result.chipId);
                    $('#devName').html(result.devName);
                    $('#devLocation').html(result.devName);                    
                    $('#supplyV').html(result.supplyVoltage + result.supplyVoltageUnit);
                    $('#resetReason').html(result.resetReason);
                    $('#freeHeap').html(result.freeHeap + result.freeHeapUnit);
                    $('#flashId').html(result.flashChipId);
                    $('#flashSize').html(result.flashChipRealSize + result.flashChipRealSizeUnit);
                    $('#flashSizeSDK').html(result.flashChipSize + result.flashChipSizeUnit);          
                    $('#flashFreq').html(result.flashChipFreq + result.flashChipFreqUnit);                    
                }
                hideLoading();
            },
            error: function () {
                setModalStatus(false);
                setModalError('Failed to retrieve available network.');                
            }
        });
    });
</script>
</html>
)=====";
