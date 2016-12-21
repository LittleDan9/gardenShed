--[[
Title: Read Temperatur & Humidiy
Created: 12/05/2016
Author: Daniel R. Little
Company: Little Squared, Inc.
Requirements: DHT22/AM2302
Purpose: This script will read data from the DHT22/AM2302 sensor using
         NodeMCUs DHT library and return the data in a nicely formated HTML string
         or a JSON formated string.
]]--

--local variable to access function outside of script in dot notation format.
local DHTBuffer = {};

--PIN should be set to the proper GPIO pin that is connected to the DHT22 data pin.
--Additionally, I added 10K Ohm Resister between 5v and Data
local DHTPin = 2;

--Convert to Fahrenheit
local convertToF = true;

--[[
    This function will return an HTML based response for quick display and
    checking functionality.
]]--
function genWeb()
    local buf = "<html>";
    buf = buf.."<head>";
    buf = buf.."<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" integrity=\"sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u\" crossorigin=\"anonymous\">";
    buf = buf.."<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\" integrity=\"sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Tx\" crossorigin=\"anonymous\"></script>";
    buf = buf.."<script src=\"https://code.jquery.com/jquery-3.1.1.min.js\" integrity=\"sha256-hVVnYaiADRTO2PzUGmuLJr8BLUSjGIZsDYGmIJLv2b8=\" crossorigin=\"anonymous\"></script>";
    buf = buf.."</head>";
    buf = buf.."<body>";
    buf = buf.."<div class=\"container\">";
    buf = buf.."<div class=\"jumbotron\">";
    buf = buf.."<h1>";
    buf = buf.."<span class=\"glyphicon glyphicon-flash\" style=\"display:inline-block;vertical-align:middle;\"></span>&nbsp;";
    buf = buf.."<span class=\"glyphicon glyphicon-cloud\" style=\"display:inline-block;vertical-align:middle;\"></span>";
    buf = buf.."<span> Garden Shed Conditions </span>";
    buf = buf.."<span class=\"glyphicon glyphicon-cloud\" style=\"display:inline-block;vertical-align:middle;\"></span>&nbsp;";    
    buf = buf.."<span class=\"glyphicon glyphicon-flash\" style=\"display:inline-block;vertical-align:middle;\"></span>";
    buf = buf.."</h1>";

    --Load Information form DHT    
    status, temp, humi, temp_dec, humi_dec = dht.read(DHTPin);
    if status == dht.OK then
        if convertToF then
            temp = temp * (9/5) + 32;
        end
        buf = buf.."<p>Current Temperature: "..temp.."&deg; F</p>";
        buf = buf.."<p>Current Humidity: "..humi.."%</p>";
    elseif status == dht.ERROR_CHECKSUM then
        buf = buf.."DHT Checksum error.";
    elseif status == dht.ERROR_TIMEOUT then
        buf = buf.."DHT timed out.";
    end
    buf = buf.."</div>";
    buf = buf.."</div>";
    buf = buf.."</body>";
    buf = buf.."</html>";
    return buf;
end
DHTBuffer.genWeb = genWeb;

--[[
    Function returns a JSON formated string. 
]]--
function genJSON()
    status, temp, humi, temp_dec, humi_dec = dht.read(DHTPin);
    if convertToF then
        temp = temp * (9/5) + 32;
    end
    
    local JSON = "";
    if status == dht.OK then
        JSON = '{"status" : "'..status..'","conditions" : {"temp" : "'..temp..'", \"humid\" : "'..humi..'"}}';
    else
        JSON = '{"status" : "'..status..'"}';
    end
    return JSON;
end
DHTBuffer.genJSON = genJSON;

return DHTBuffer;
