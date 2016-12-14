--[[
    Title: Inialization of the ESP8266.
    Created: 12-10-2016
    Author: Daniel R. Little
    Company: Little Squared, Inc.
    Description: This will script will start the ESP8266's WiFi and connect to the specified WiFi SSID/Password 
                 Once connected, if useStatic is true it will assin the static network variables to the interface.
    Defaults: WiFi SSID and Password must be specified
             Hostname is optional. If omitted, set to "[hostname]"
             Static Interface is disabled, if enabled satic variables must be set. 
]]--
local ssid = "[ssid]";
local wifiPass = "[password]";

local hostname = "[hostname]";

local useStatic = false;
local staticIP = "[static-ip]";
local staticNetMask = "[static-netmask]";
local staticGateway = "[static-gateway]";


function startup()
    if file.open("init.lua") == nil then
        print("init.lua deleted of renamed");
    else
        file.close("init.lua");
        print("Starting Main Routine (main.lua)");
        dofile("DHTNetServer.lua");
    end
end


local success = true;

--Configuration WiFi for station access with the configuration specified above
print("Connecting to WiFi");
wifi.setmode(wifi.STATION);
success = wifi.sta.config(ssid, wifiPass);
--If successfuly connected, obtain/set WiFi configuration.
if success then
    --Run a timer every second until IP Address has been obtained.
    tmr.alarm(1, 1000, 1, function()
        if wifi.sta.getip() == nil and useStatic == false then
            print("Waiting for IP Address (DHCP)...");
        else --Either we have an IP from DHCP or we are using static configuration.
            tmr.stop(1);
            if not useStatic then
                print("WiFi Connection Established, IP Address: " .. wifi.sta.getip());
            else
                print("Setting static IP network information.");
                success = wifi.sta.setip({ip=staticIP,netmask=staticNetMask,gateway=staticGateway});
                if success then
                    print("WiFi connection static paramters set:")
                    print("IP Address: ".. wifi.sta.getip());
                    print("Netmask: "..staticNewMask);
                    print("Gateway: "..staticGateway);
                else
                    print("Unable to set static configuration. Check paramters and station connection information.")
                end
            end
            --Specifiy a custom host name.
            if hostname ~= "[hostname]" then
                success = wifi.sta.sethostname(hostname);
                if success then
                    print("Hostname set to: "..wifi.sta.gethostname())
                else
                    print("Unable to set hostname!")
                end
            end
            print("You have 3 seconds to abort");
            print("Waiting...");
            if success then
                tmr.alarm(0, 3000, 0, startup);
            else
                print("One of more issues occured. Please check console for errors and verify any configuration variables.")
            end                
        end
    end);
else
    print("Unable to connect to specific access point ("..ssid.."). Please check confniguration and try again.");
end