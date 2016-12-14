
function getShedConditions(){
    var rotateGlyph = null;
    $.ajax({
        beforeSend: function(){
            $('#btnRefreshGlyph').css('color', 'green');
            rotateGlyph = setInterval(function(){$('#btnRefreshGlyph').rotate()}, 100)
        },
        method: 'GET',
        url: '/Conditions/1',
        dataType: 'JSON',
        success: function(data){
            if(!data || data.status < 0){
                //Try to get from DB?
                $('#temp').html('<i class="icon wi wi-na"></i>');
                $('#humid').html('<i class="icon wi wi-na"></i>');
            }else{
                $('#lstUpd').remove();
                getDateTimeString(data.observed, function(dteString){
                    $('#observationTime').html(dteString);
                });                
                $('#temp').html(data.temp + '&deg;').parent('span').effect("highlight", {}, 1000);
                $('#humid').html(data.humidity + '%').parent('span').effect("highlight", {}, 1000);;
                if(!data.isLive){
                    var oDate_ms = new Date(data.observed).getTime();
                    var now_ms = new Date().getTime();

                    var oneDay = 1000*60*60*24;
                    var diff = now_ms - oDate_ms;
                    var days = Math.round(diff/oneDay);
                    var hours = Math.floor((diff/(oneDay/24)))-(days*24);
                    var mins = Math.floor((diff/(oneDay/24/60))) - ((hours*60) + (days*24*60));
                    var lstUpd = $('<p id="lstUpd"/>').html('<strong>Last Updated: ' + 
                                                           (days > 0 ? days.toString() + ' Days ' : '') + 
                                                           (hours > 0 ? (hours == 1 ? hours.toString() + ' Hour ' : hours.toString() + ' Hours ') : ' and ') + 
                                                           (mins > 0 ? + (mins == 1 ? mins.toString() + ' Minute' : mins.toString() + ' Minutes') : '') + ' Ago!</strong>')
                    $('#observationTime').after(lstUpd);
                    $(lstUpd ).effect("highlight", {}, 8000);

                }
            }
        },
        complete: function(){
            clearInterval(rotateGlyph);
            $('#btnRefreshGlyph').css('color', 'black');
            $('#btnRefreshGlyph').data('angle', 0);
            $('#btnRefreshGlyph').rotate();
            $('#btnRefreshGlyph').data('angle', 0);
        }
    });
}

function getExtConditions(){
    $.ajax({
        method: 'GET',
        url: 'http://api.wunderground.com/api/a75da485666047dd/geolookup/conditions/q/pws:KORPORTL166.json',
        dataType: 'JSON',
        success: function(data){
            //console.log(data);
            if(data.current_observation)
            {
                $('#extStationID').html('<a href="https://www.wunderground.com/cgi-bin/findweather/getForecast?query=pws:KORPORTL166&MR=1&apiref=cd49dbb7619f6522">' + data.current_observation.station_id + '</a>');
                getDateTimeString(data.current_observation.observation_time_rfc822, function(dteString){
                    $('#extObservationTime').html(dteString);
                });
                $('#extCondition').html(data.current_observation.weather);
                var icon = data.current_observation.icon;
                if(data.current_observation.icon_url.indexOf('nt_' + data.current_observation.icon) > -1)
                    icon = 'nt_' + icon;
                var result = $.grep(iconsxref, function(e){return e.wuIcon == icon})
                var iconClass = 'wi-na';
                if(result.length === 0)
                    iconClass = 'wi-na';
                else if(result.length >= 1)
                    iconClass = result[0].wiIcon;

                $('#extCondIcon').addClass(iconClass);
                $('#extTemp').html(data.current_observation.feelslike_f + '&deg;');
                $('#extHumid').html(data.current_observation.relative_humidity);
                $('#extWind').html(data.current_observation.wind_string);

                $('#outside').removeClass('hidden');
                $('#wuLogo').removeClass('hidden');
            }
        }
    });
}

function getDateTimeString(date, callback){
    $.ajax({
        method: 'GET',
        url: '/DateString?dt=' + date,
        dataType: 'JSON',
        success: function(result){
            callback(result.dtString);
        }
    })
}