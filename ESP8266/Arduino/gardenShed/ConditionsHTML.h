const char ConditionsHTML[] PROGMEM = R"=====(
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
    <script src="https://code.jquery.com/ui/1.12.1/jquery-ui.min.js" integrity="sha256-VazP97ZCwtekAsvgPBSUwPFKdrwD3unUfSGVYrahUqU=" crossorigin="anonymous"></script>    
    <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js" integrity="sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa" crossorigin="anonymous"></script>
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css" integrity="sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u" crossorigin="anonymous">   
    <link rel="stylesheet" href="https://cdn.rawgit.com/erikflowers/weather-icons/master/css/weather-icons.min.css">
    <script type="text/javascript">
        function showLoading() {
            $('#status').html("");
            $('#errorMessage').html("");
            $('#loadingFooter').addClass('hidden');
            $('#loadingBar').removeClass('hidden');
            $('#processingModal').modal({backdrop: 'static', keyboard: false, show: true});
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
                        <li role="presentation"><a href="/wifi">WiFi Settings</a></li>
                        <li role="presentation"><a href="/config">System Configuration</a></li>
                        <li role="presentation" class="active"><a href="/conditions">Current Conditions</a></li>
                        <li role="presentation"><a href="/about">About</a></li>
                    </ul>
                </div>
            </div>        
            <div class="panel-body content">
                <div class="row">
                    <div class="col-sm-offset-1 col-xs-10">
                        <h2>Conditions 
                            <button class="btn btn-default" id="btnRefresh">
                                <span style="display: inline-block; color: rgb(0, 0, 0); transform: rotate(305deg);" id="btnRefreshGlyph" class="glyphicon glyphicon-refresh"></span>
                            </button>
                        </h2>
                        <h3 style="margin-left:20px;"><i class="icon wi wi-thermometer"></i>&nbsp;Temperature:&nbsp;<span id="tempValue"></span></h3>
                        <h3 style="margin-left:20px;"><i class="icon wi wi-humidity"></i>&nbsp;Humidity:&nbsp;<span id="humidValue"></span></h3>
                    </div>
                </div>
            </div>
        </div>
    </div>
</body>
<script type="text/javascript">

jQuery.fn.rotate = function() {
    degrees = $(this).data("angle");
    $(this).css({'-webkit-transform' : 'rotate('+ degrees +'deg)',
                '-moz-transform' : 'rotate('+ degrees +'deg)',
                '-ms-transform' : 'rotate('+ degrees +'deg)',
                'transform' : 'rotate('+ degrees +'deg)'});
    $(this).data("angle", degrees + 5);                 
};

$(function () {
    $('#btnRefreshGlyph').data("angle", 0);
    getConditions(true);
});

$('#btnRefresh').on('click', function(){getConditions(false)});

function getConditions(hideModal){
    var rotateGlyph = null;
    $.ajax({
        beforeSend: function(){
            $('#btnRefreshGlyph').css('color', 'green');
            rotateGlyph = setInterval(function(){$('#btnRefreshGlyph').rotate()}, 100)
        },
        method: 'GET',
        url: '/api/getConditions',
        dataType: 'JSON',
        success: function(data){
            if(!data){
                $('#tempValue').html('<i class="icon wi wi-na"></i>');
                $('#humidValue').html('<i class="icon wi wi-na"></i>');
            }else{
                $('#tempValue').html(data.temperature + '&deg;').parent('h3').effect("highlight", {}, 1000);
                $('#humidValue').html(data.humidity + '%').parent('h3').effect("highlight", {}, 1000);;
            }
            if(hideModal)
                hideLoading();
        },
        complete: function(){
            clearInterval(rotateGlyph);
            $('#btnRefreshGlyph').css('color', 'black');
            $('#btnRefreshGlyph').finish();
        }
    });
}    
</script>
</html>
)=====";
