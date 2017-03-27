const char SysConfigJS[] PROGMEM = R"=====(
var colors =  {
    "all" : [
        {"name" : "Black", "html" : "#000000", "hex" : "0x0000"},
        {"name" : "Navy", "html" : "#000080", "hex" : "0x000F"},
        {"name" : "Dark Green", "html" : "#008000", "hex" : "0x03E0"},
        {"name" : "Dark Cyan", "html" : "#008080", "hex" : "0x03EF"},
        {"name" : "Maroon", "html" : "#800000", "hex" : "0x7800"},
        {"name" : "Purple", "html" : "#800080", "hex" : "0x780F"},
        {"name" : "Olive", "html" : "#808000", "hex" : "0x7BE0"},
        {"name" : "Light Grey", "html" : "#C0C0C0", "hex" : "0xC618"},
        {"name" : "Dark Grey", "html" : "#808080", "hex" : "0x7BEF"},
        {"name" : "Blue", "html" : "#0000FF ", "hex" : "0x001F"},
        {"name" : "Green", "html" : "#00FF00", "hex" : "0x07E0"},
        {"name" : "Cyan", "html" : "#00FFFF", "hex" : "0x07FF"},
        {"name" : "Red", "html" : "#FF0000", "hex" : "0xF800"},
        {"name" : "Magenta", "html" : "#FF00FF", "hex" : "0xF81F"},
        {"name" : "Yellow", "html" : "#FFFF00", "hex" : "0xFFE0"},
        {"name" : "White", "html" : "#FFFFFF", "hex" : "0xFFFF"},
        {"name" : "Orange", "html" : "#FFA500 ", "hex" : "0xFD20"},
        {"name" : "Special Orange", "html" : "#F49542 ", "hex" : "0xF4A8"},
        {"name" : "Green Yellow", "html" : "#ADFF00", "hex" : "0xAFE5"},
        {"name" : "Pink", "html" : "#FF80C0", "hex" : "0xFC18"}
    ]
}

function selectColor(color, index) {
    $('#txtClockColor').val(colors.all[index].hex);
    $('#selectedColor').html($(color).html());
}

function selectBkgColor(color, index) {
    $('#txtClockBkgColor').val(colors.all[index].hex);
    $('#selectedBkgColor').html($(color).html());
}

$(function () {
    var colorSelectors = ["Color", "BkgColor"];
    //Populate the Clock Text Colors
    for(i = 0; i < colorSelectors.length; i++){
        var liColors = "";
        for (var index in colors.all) {
            liColors += '<li style="cursor:pointer">';
            //console.log(colorSelectors[i]);
            liColors += '<a id="' + colorSelectors[i] + index + '" onclick="select' + colorSelectors[i] + '(this, ' + index + ');">'
            liColors += '&nbsp;<span class="color" style="border: 1px solid black; background-color:' + colors.all[index].html + '">&nbsp;&nbsp;&nbsp;&nbsp;</span>';
            liColors += '<span class="value">&nbsp;' + colors.all[index].name + '</span>'
            liColors += '</a></li>';
        }
        $('#ddl' + colorSelectors[i] + 'Items').html(liColors);
    }
    //END Clock Text Color

  $('[data-toggle="popover"]').popover();
    $.ajax({
        url: '/api/getSysSettings',
        data: "",
        dataType: 'json',
        success: function (result) {
            console.log(result);
            if (result != null) {
                $('#txtDeviceName').val(result.deviceName);
                $('#txtDeviceLocation').val(result.deviceLocation);
                $('[name="displayClockOption"]').prop('checked', false);
                $('[name="displayClockOption"][value="' + result.displayClock + '"]').prop('checked', true);
                $('[name="displayDateOption"]').prop('checked', false);
                $('[name="displayDateOption"][value="' + result.displayDate + '"]').prop('checked', true);                
                $('#ddlClockType option[value="' + result.clockType + '"]').prop("selected", true);
                if(result.clockType == 1){
                    $('#divClockColor').collapse('show');
                }else{
                    $('#divClockColor').collapse('hide');
                }
                $('#txtClockColor').val(result.clockColor)
                //Set the correct DDL Item
                for (var index in colors.all) {
                    if(parseInt(colors.all[index].hex, 16) == result.clockColor){
                        selectColor($('#Color' + index), index);
                        break;
                    }                    
                }
                $('#txtClockBkgColor').val(result.clockBackgroundColor)
                //Set the correct DDL Item
                for (var index in colors.all) {
                    if(parseInt(colors.all[index].hex, 16) == result.clockBackgroundColor){
                        selectBkgColor($('#BkgColor' + index), index);
                        break;
                    }                    
                }      
                $('[name="leadingZeroOption"]').prop('checked', false);   
                $('[name="leadingZeroOption"][value="' + result.leadingZero + '"]').prop('checked', true);       
                $('#txtInterval').val(result.interval);
                $('#ddlScreenRotation option[value="' + result.screenRotation + '"]').prop("selected", true);
                $('#txtUsername').val(result.username);
                $('#txtHTTPPort').val(result.httpPort);
                $('#txtTCPPort').val(result.tcpPort);
                $('#txtRoomTemp').val(result.roomTemperature);
                $('[name="formatOption"]').prop('checked', false);
                $('[name="formatOption"][value="' + result.deleteFileSystem + '"]').prop('checked', true);
            }
            hideLoading();
            $('#btnSave').prop('disabled', false);
        },
        error: function () {
            setModalStatus(false);
            setModalError('Failed to retrieve system configuration. Refresh page to try again.');                
        }
    });
});

$('#formatWarning').on('mouseover', function(){
    $('#formatWarning').popover('show');
});
$('#formatWarning').on('mouseout', function(){
    setInterval(function(){$('#formatWarning').popover('hide'); clearInterval()}, 10000);   
});

$('#rotationWarning').on('mouseover', function(){
    $('#rotationWarning').popover('show');
});
$('#rotationWarning').on('mouseout', function(){
    setInterval(function(){$('#rotationWarning').popover('hide'); clearInterval()}, 10000);   
});

$(document).on('click', function(){
    $('#formatWarning').popover('hide');
    $('#rotationWarning').popover('hide');
});

$('#btnSave').on('click', function(){
    showLoading();
    var toSend = {
        DeviceName: $('#txtDeviceName').val(),
        DeviceLocation: $('#txtDeviceLocation').val(),
        Username: $('#txtUsername').val(),
        Password: $('#txtPassword').val(),
        Interval: $('#txtInterval').val(),
        HTTPPort: $('#txtHTTPPort').val(),
        TCPPort: $('#txtTCPPort').val(),
        ClockType: $('#ddlClockType option:selected').val(),
        ClockColor: parseInt($('#txtClockColor').val(), 16),
        ClockBackgroundColor: parseInt($('#txtClockBkgColor').val(), 16),
        RoomTemperature: $('#txtRoomTemp').val(),
        ScreenRotation: $('#ddlScreenRotation option:selected').val(),
        DeleteFileSystem:$('[name="formatOption"]:checked').val(),
        DisplayClock: $('[name="displayClockOption"]:checked').val(),
        DisplayDate: $('[name="displayDateOption"]:checked').val(),
        LeadingZero: $('[name="leadingZeroOption"]:checked').val()
    };
    console.log(toSend);
    $.ajax({
        url: '/config/save',
        data: toSend,
        dataType: 'json',
        success: function (data) {
            $('#loadingBar').addClass('hidden');
            setModalStatus(data.success);
            if (!data.success) {
                setModalError(data.error);
            }
            $('#loadingFooter').removeClass('hidden');
        },
        error: function () {
            $('#loadingBar').addClass('hidden');
            setModalStatus(false);
            setModalError("Failed to submit changes to device.");
            $('#loadingFooter').removeClass('hidden');
        }

    });
});

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
)=====";
