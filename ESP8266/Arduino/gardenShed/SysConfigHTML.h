const char SysConfigHTML[] PROGMEM = R"=====(
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
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js"></script>
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js" integrity="sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa" crossorigin="anonymous"></script>
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css" integrity="sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u" crossorigin="anonymous"> 
    <script type="text/javascript">
        function showLoading() {
            $('#status').html("");
            $('#errorMessage').html("");
            $('#loadingFooter').addClass('hidden');
            $('#loadingBar').removeClass('hidden');
            //$('#processingModal').modal({backdrop: 'static', keyboard: false, show: true});
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
        <script type="text/javascript">
            showLoading();
        </script>
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
                        <li role="presentation"><a href="/wifi">WiFi Settings</a></li>
                        <li role="presentation" class="active"><a href="/config">System Configuration</a></li>
                        <li role="presentation"><a href="/conditions">Current Conditions</a></li>
                        <li role="presentation"><a href="/about">About</a></li>
                    </ul>
                </div>
            </div>        
            <div class="panel-body content">
                <div class="row">
                    <div class="col-sm-offset-1 col-xs-10">
                        <form id="rootForm" class="form-horizontal">
                            <!--System Settings Panel-->
                            <div class="panel panel-primary">
                                <div class="panel-heading">
                                    <h2>System Configuration</h2>
                                </div>
                                <div class="panel-body">
                                    <div class="row">
                                        <div class="col-xs-10 col-xs-offset-1">
                                            <div class="panel panel-info">
                                                <div class="panel-heading">
                                                    <h4>Device Administration</h4>
                                                </div>
                                                <div class="panel-body">
                                                    <div class="col-xs-10 col-xs-offset-1">
                                                        <div class="form-group">
                                                            <label for="txtDeviceName" class="control-label">Device Name</label>
                                                            <input type="text" class="form-control" id="txtDeviceName" placeholder="Big Blue" value="" maxlength="30">
                                                        </div>
                                                        <div class="form-group">
                                                            <label for="txtDeviceLocation" class="control-label">Device Location</label>
                                                            <input type="text" class="form-control" id="txtDeviceLocation" placeholder="Bedroom" value="" maxlength="30">
                                                        </div>
                                                        <div class="form-group">
                                                            <label for="txtRoomTemp" class="control-label">Room Temperature</label>
                                                            <div class="input-group">
                                                                <input type="text" class="form-control" id="txtRoomTemp" placeholder="##" maxlength="6" value="">
                                                                <div class="input-group-addon">&deg;</div>
                                                            </div>                                                                                                                        
                                                        </div>                                                          
                                                        <div class="form-group">
                                                            <label for="txtUsername" class="control-label">Device Username</label>
                                                            <input type="text" class="form-control" id="txtUsername" placeholder="" value="" maxlength="30">
                                                        </div>
                                                        <div class="form-group">
                                                            <label for="txtPassword" class="control-label">Device Password</label>
                                                            <input type="password" class="form-control" id="txtPassword" placeholder="" value="" maxlength="30">
                                                        </div>   
                                                        <div class="form-group">
                                                            <label for="rdoFormat" class="control-label">Format Filesystem 
                                                                <span id="formatWarning" class="glyphicon glyphicon-warning-sign text-danger" 
                                                                    style="cursor: pointer"
                                                                    data-container="body" 
                                                                    data-toggle="popover" 
                                                                    data-placement="right" 
                                                                    data-html="true"
                                                                    data-content="This will <span style='font-weight:bold; color:#d9534f;'>DELETE</span> all downloaded resources from the file system. Without a connection to the Internet you will not be able to obtain the conditions icons.<br/><br/> This <b style='color:#5cb85c;'>WILL NOT</b> erase WiFi or System Configuration."
                                                                    data-title="File System Format Warning"></span> 
                                                            </label><br/>
                                                            <label class="radio-inline">
                                                                <input type="radio" name="formatOption" id="rdoFormatYes" value="true"> Yes
                                                            </label>
                                                            <label class="radio-inline">
                                                                <input type="radio" name="formatOption" id="rdoFormatNo" value="false" checked> No (Default)
                                                            </label>                                         
                                                        </div>                                                            
                                                    </div>
                                                </div>                                            
                                            </div>
                                            <div class="panel panel-info">
                                                <div class="panel-heading">
                                                    <h4>Display Settings</h4>
                                                </div>
                                                <div class="panel-body">
                                                    <div class="col-xs-10 col-xs-offset-1">
                                                        <div class="form-group">
                                                            <label for="ddlScreenRotation" class="control-label">Screen Rotation&nbsp;
                                                                <span id="rotationWarning" class="glyphicon glyphicon-warning-sign text-danger" 
                                                                    style="cursor: pointer"
                                                                    data-container="body" 
                                                                    data-toggle="popover" 
                                                                    data-placement="right" 
                                                                    data-html="true"
                                                                    data-content="Portrait is an option, however it is not yet supported. Selecting a portrait rotation will result in undesired visuals."
                                                                    data-title="Screen Rotation Warning"></span> 
                                                            </label>
                                                            <select class="form-control" id="ddlScreenRotation">
                                                                <option value="-1">Select a Screen Rotation</option>
                                                                <option value="1">0&deg; Landscape (Default)</option>
                                                                <option value="0">90&deg; Portrait</option>
                                                                <option value="3">180&deg; Landscape</option>
                                                                <option value="2">270&deg; Portrait</option>
                                                            </select>
                                                        </div>                                                                                                                                                                                                
                                                        <div class="form-group">
                                                            <label for="txtInterval" class="control-label">Clock / Conditions Time Interval in Seconds (Default: 10)</label>
                                                            <input type="text" class="form-control" id="txtInterval" placeholder="##" maxlength="6">
                                                        </div>
                                                    </div>
                                                </div>
                                             </div> 
                                            <div class="panel panel-info">
                                                <div class="panel-heading">
                                                    <h4>Clock Settings</h4>
                                                </div>
                                                <div class="panel-body">
                                                    <div class="col-xs-10 col-xs-offset-1">
                                                        <div class="form-group">
                                                            <label for="rdoDisplayClock" class="control-label">Display Clock</label><br/>
                                                            <label class="radio-inline">
                                                                <input type="radio" name="displayClockOption" id="rdoDisplayClockYes" value="true" checked> Yes (Default)
                                                            </label>
                                                            <label class="radio-inline">
                                                                <input type="radio" name="displayClockOption" id="rdoDisplayClockNo" value="false"> No
                                                            </label>                                         
                                                        </div>
                                                        <div class="form-group">
                                                            <label for="ddlClockType" class="control-label">Clock Type&nbsp; </label>
                                                            <select class="form-control" id="ddlClockType" onchange="$(this).val() == 1 ? $('#divClockColor').collapse('show') : $('#divClockColor').collapse('hide');">
                                                                <option value="-1">Select a Clock Face</option>
                                                                <option value="0">Analog</option>
                                                                <option value="1">7-Segment</option>
                                                            </select>
                                                        </div>  
                                                        <div id="divClockColor" class="collapse">
                                                            <div class="form-group">
                                                                <label for="ddlClockColor" class="control-label">Clock Color&nbsp; </label>
                                                                <div class="dropdown">
                                                                    <button id="dLabel" class="btn btn-default" type="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
                                                                        <span id="selectedColor">Select a Color</span>
                                                                        <span class="caret"></span>
                                                                    </button>
                                                                    <ul id="ddlColorItems" class="dropdown-menu" aria-labelledby="dLabel"></ul>
                                                                    <input type="hidden" id="txtClockColor" value=""/>
                                                                </div>
                                                            </div>
                                                            <div class="form-group">
                                                                <label for="ddlClockBkgColor" class="control-label">Clock Background Color&nbsp; </label>
                                                                <div class="dropdown">
                                                                    <button id="dLabel" class="btn btn-default" type="button" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
                                                                        <span id="selectedBkgColor">Select a Color</span>
                                                                        <span class="caret"></span>
                                                                    </button>
                                                                    <ul id="ddlBkgColorItems" class="dropdown-menu" aria-labelledby="dLabel"></ul>
                                                                    <input type="hidden" id="txtClockBkgColor" value=""/>
                                                                </div>
                                                            </div>                                                            
                                                        </div>
                                                        <div class="form-group">
                                                            <label for="rdoDisplayDate" class="control-label">Display Date</label><br/>
                                                            <label class="radio-inline">
                                                                <input type="radio" name="displayDateOption" id="rdoDisplayDateYes" value="true" checked> Yes
                                                            </label>
                                                            <label class="radio-inline">
                                                                <input type="radio" name="displayDateOption" id="rdoDisplayDateNo" value="false"> No (Default)
                                                            </label>                                         
                                                        </div>  
                                                        <div class="form-group">
                                                            <label for="rdoLeadingZero" class="control-label">Leading Zero on Hour</label><br/>
                                                            <label class="radio-inline">
                                                                <input type="radio" name="leadingZeroOption" id="rdoLeadingZeroYes" value="true" checked> Yes
                                                            </label>
                                                            <label class="radio-inline">
                                                                <input type="radio" name="leadingZeroOption" id="rdoLeadingZeroNo" value="false"> No (Default)
                                                            </label>                                         
                                                        </div>                                                          
                                                    </div> 
                                                </div>
                                            </div>
                                            <div class="panel panel-info">
                                                <div class="panel-heading">
                                                    <h4>Web Server Settings</h4>
                                                </div>
                                                <div class="panel-body">
                                                    <div class="col-xs-10 col-xs-offset-1">
                                                        <div class="form-group">
                                                            <label for="txtHTTPPort" class="control-label">HTTP Port (Default: 80)</label>
                                                            <input type="text" class="form-control" id="txtHTTPPort" placeholder="##" maxlength="6">
                                                        </div>                                                                                      
                                                        <div class="form-group">
                                                            <label for="txtTCPPort" class="control-label">TCP Port (Default: 4210)</label>
                                                            <input type="text" class="form-control" id="txtTCPPort" placeholder="####" maxlength="6">
                                                        </div>
                                                    </div>
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
<script src="/js/sysConfig.js" type="text/javascript"></script>
</html>
)=====";
