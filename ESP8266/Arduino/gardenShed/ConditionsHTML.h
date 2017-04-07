const char ConditionsHTML[] PROGMEM = R"=====(<!DOCTYPE html><style>#iconsLeft .glyphicon-flash,#iconsRight .glyphicon-flash{color:#ff0;-webkit-text-stroke:1px #000;text-shadow:3px 3px 0 #000,-1px -1px 0 #000,1px -1px 0 #000,-1px 1px 0 #000,1px 1px 0 #000}#iconsLeft .glyphicon-cloud,#iconsRight .glyphicon-cloud{color:#fff;-webkit-text-stroke:1px #000;text-shadow:3px 3px 0 #000,-1px -1px 0 #000,1px -1px 0 #000,-1px 1px 0 #000,1px 1px 0 #000}</style><script src=https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js></script><script src=https://code.jquery.com/ui/1.12.1/jquery-ui.min.js crossorigin=anonymous integrity="sha256-VazP97ZCwtekAsvgPBSUwPFKdrwD3unUfSGVYrahUqU="></script><script src=https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js crossorigin=anonymous integrity=sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa></script><link href=https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css rel=stylesheet crossorigin=anonymous integrity=sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u><link href=https://cdn.rawgit.com/erikflowers/weather-icons/master/css/weather-icons.min.css rel=stylesheet><script>function showLoading(){$("#status").html(""),$("#errorMessage").html(""),$("#loadingFooter").addClass("hidden"),$("#loadingBar").removeClass("hidden"),$("#processingModal").modal({backdrop:"static",keyboard:!1,show:!0})}function hideLoading(){$("#processingModal").modal("hide"),$("#loadingFooter").removeClass("hidden"),$("#loadingBar").addClass("hidden")}</script><meta content="width=device-width,initial-scale=1" name=viewport><body style=margin:20px><div class=container><div class="fade modal" id=processingModal role=dialog tabindex=-1><div class=modal-dialog role=document><div class=modal-content><div class=modal-header><h4 class=modal-title>Processing - Please Wait</h4></div><div class=modal-body><div class=progress id=loadingBar><div class="active progress-bar progress-bar-striped" role=progressbar aria-valuemax=100 aria-valuemin=0 aria-valuenow=100 style=width:100%><span class=sr-only>Please Wait - Loading</span></div></div><div id=status></div><div id=errorMessage></div></div><div class=modal-footer id=loadingFooter><button class="btn btn-primary" data-dismiss=modal type=button>OK</button></div></div></div></div><script>showLoading()</script><div class="panel panel-default"><div class="jumbotron panel-heading text-center"><div class=heading style=display:inline;vertical-align:middle><div class=row><div class="col-sm-4 col-md-3" id=iconsLeft><h1><span class="glyphicon glyphicon-flash"></span> <span class="glyphicon glyphicon-cloud"></span></h1></div><div class="col-sm-4 col-md-6" id=title><h1><span>Device Setup</span></h1></div><div class="col-sm-4 col-md-3" id=iconsRight><h1><span class="glyphicon glyphicon-cloud"></span> <span class="glyphicon glyphicon-flash"></span></h1></div></div></div></div><div class=row><div class="col-sm-offset-1 col-xs-10"><ul class="nav nav-tabs"><li role=presentation><a href=/ >Home</a><li role=presentation><a href=/wifi>WiFi Settings</a><li role=presentation><a href=/config>System Configuration</a><li role=presentation class=active><a href=/conditions>Current Conditions</a><li role=presentation><a href=/about>About</a></ul></div></div><div class="content panel-body"><div class=row><div class="col-sm-offset-1 col-xs-10"><h2>Conditions <button class="btn btn-default" id=btnRefresh><span class="glyphicon glyphicon-refresh" id=btnRefreshGlyph style=display:inline-block;color:#000;transform:rotate(305deg)></span></button></h2><h3 style=margin-left:20px><i class="icon wi wi-thermometer"></i> Temperature: <span id=tempValue></span></h3><h3 style=margin-left:20px><i class="icon wi wi-humidity"></i> Humidity: <span id=humidValue></span></h3></div></div></div></div></div><script>function getConditions(e){var t=null;$.ajax({beforeSend:function(){$("#btnRefreshGlyph").css("color","green"),t=setInterval(function(){$("#btnRefreshGlyph").rotate()},100)},method:"GET",url:"/api/getConditions",dataType:"JSON",success:function(t){t?($("#tempValue").html(t.temperature+"&deg;").parent("h3").effect("highlight",{},1e3),$("#humidValue").html(t.humidity+"%").parent("h3").effect("highlight",{},1e3)):($("#tempValue").html('<i class="icon wi wi-na"></i>'),$("#humidValue").html('<i class="icon wi wi-na"></i>')),e&&hideLoading()},complete:function(){clearInterval(t),$("#btnRefreshGlyph").css("color","black"),$("#btnRefreshGlyph").finish()}})}jQuery.fn.rotate=function(){degrees=$(this).data("angle"),$(this).css({"-webkit-transform":"rotate("+degrees+"deg)","-moz-transform":"rotate("+degrees+"deg)","-ms-transform":"rotate("+degrees+"deg)",transform:"rotate("+degrees+"deg)"}),$(this).data("angle",degrees+5)},$(function(){$("#btnRefreshGlyph").data("angle",0),getConditions(!0)}),$("#btnRefresh").on("click",function(){getConditions(!1)})</script>)=====";