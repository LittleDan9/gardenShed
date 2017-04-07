const char HomeHTML[] PROGMEM = R"=====(<!DOCTYPE html><style>#iconsLeft .glyphicon-flash,#iconsRight .glyphicon-flash{color:#ff0;-webkit-text-stroke:1px #000;text-shadow:3px 3px 0 #000,-1px -1px 0 #000,1px -1px 0 #000,-1px 1px 0 #000,1px 1px 0 #000}#iconsLeft .glyphicon-cloud,#iconsRight .glyphicon-cloud{color:#fff;-webkit-text-stroke:1px #000;text-shadow:3px 3px 0 #000,-1px -1px 0 #000,1px -1px 0 #000,-1px 1px 0 #000,1px 1px 0 #000}.detailItem{font-size:18pt;margin:10px 0}</style><script src=https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js></script><script src=https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js crossorigin=anonymous integrity=sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa></script><link crossorigin=anonymous href=https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css integrity=sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u rel=stylesheet><script>function showLoading(){$("#status").html(""),$("#errorMessage").html(""),$("#loadingFooter").addClass("hidden"),$("#loadingBar").removeClass("hidden"),$("#processingModal").modal({backdrop:"static",keyboard:!1,show:!0})}function hideLoading(){$("#processingModal").modal("hide"),$("#loadingFooter").removeClass("hidden"),$("#loadingBar").addClass("hidden")}</script><meta content="width=device-width,initial-scale=1" name=viewport><body style=margin:20px><div class=container><div class="fade modal" id=processingModal role=dialog tabindex=-1><div class=modal-dialog role=document><div class=modal-content><div class=modal-header><h4 class=modal-title>Processing - Please Wait</h4></div><div class=modal-body><div class=progress id=loadingBar><div class="active progress-bar progress-bar-striped" role=progressbar aria-valuemax=100 aria-valuemin=0 aria-valuenow=100 style=width:100%><span class=sr-only>Please Wait - Loading</span></div></div><div id=status></div><div id=errorMessage></div></div><div class=modal-footer id=loadingFooter><button class="btn btn-primary" data-dismiss=modal type=button>OK</button></div></div></div></div><script>showLoading()</script><div class="panel panel-default"><div class="jumbotron panel-heading text-center"><div class=heading style=display:inline;vertical-align:middle><div class=row><div class="col-sm-4 col-md-3" id=iconsLeft><h1><span class="glyphicon glyphicon-flash"></span> <span class="glyphicon glyphicon-cloud"></span></h1></div><div class="col-sm-4 col-md-6" id=title><h1><span>Device Setup</span></h1></div><div class="col-sm-4 col-md-3" id=iconsRight><h1><span class="glyphicon glyphicon-cloud"></span> <span class="glyphicon glyphicon-flash"></span></h1></div></div></div></div><div class=row><div class="col-sm-offset-1 col-xs-10"><ul class="nav nav-tabs"><li role=presentation class=active><a href=/ >Home</a><li role=presentation><a href=/wifi>WiFi Settings</a><li role=presentation><a href=/config>System Configuration</a><li role=presentation><a href=/conditions>Current Conditions</a><li role=presentation><a href=/about>About</a></ul></div></div><div class="panel-body content"><div class=row><div class="col-sm-offset-1 col-xs-10 detailItem"><div class="panel panel-default"><div class=panel-body><div class=row><div class="col-sm-offset-1 col-xs-10 detailItem"><span class="glyphicon glyphicon-tag"></span> Chip ID: <span id=chipId></span></div></div><div class=row><div class="col-sm-offset-1 col-xs-10 detailItem"><span class="glyphicon glyphicon-user"></span> Device Name: <span id=devName></span></div></div><div class=row><div class="col-sm-offset-1 col-xs-10 detailItem"><span class="glyphicon glyphicon-map-marker"></span> Device Location: <span id=devLocation></span></div></div><hr><div class=row><div class="col-sm-offset-1 col-xs-10 detailItem"><span class="glyphicon glyphicon-flash"></span> Supply Voltage: <span id=supplyV></span></div></div><div class=row><div class="col-sm-offset-1 col-xs-10 detailItem"><span class="glyphicon glyphicon-off"></span> Last Reset Reason: <span id=resetReason></span></div></div><hr><div class=row><div class="col-sm-offset-1 col-xs-10 detailItem"><span class="glyphicon glyphicon-compressed"></span> Free Heap: <span id=freeHeap></span></div></div><hr><div class=row><div class="col-sm-offset-1 col-xs-10 detailItem"><span class="glyphicon glyphicon-tag"></span> Flash Chip ID: <span id=flashId></span></div></div><div class=row><div class="col-sm-offset-1 col-xs-10 detailItem"><span class="glyphicon glyphicon-floppy-disk"></span> Flash Chip Size: <span id=flashSize></span></div></div><div class=row><div class="col-sm-offset-1 col-xs-10 detailItem"><span class="glyphicon glyphicon-floppy-disk"></span> Flash Chip Size (SDK): <span id=flashSizeSDK></span></div></div><div class=row><div class="col-sm-offset-1 col-xs-10 detailItem"><span class="glyphicon glyphicon-random"></span> Flash Chip Frequency: <span id=flashFreq></span></div></div></div></div></div></div></div></div></div><script>$(function(){$.ajax({url:"/api/getDeviceDetails",data:"",dataType:"json",success:function(e){null!=e&&($("#chipId").html(e.chipId),$("#devName").html(e.deviceName),$("#devLocation").html(e.deviceLocation),$("#supplyV").html(e.supplyVoltage+e.supplyVoltageUnit),$("#resetReason").html(e.resetReason),$("#freeHeap").html(e.freeHeap+e.freeHeapUnit),$("#flashId").html(e.flashChipId),$("#flashSize").html(e.flashChipRealSize+e.flashChipRealSizeUnit),$("#flashSizeSDK").html(e.flashChipSize+e.flashChipSizeUnit),$("#flashFreq").html(e.flashChipFreq+e.flashChipFreqUnit)),hideLoading()},error:function(){setModalStatus(!1),setModalError("Failed to retrieve available network.")}})})</script>)=====";
