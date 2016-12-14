--[[
    Title: Run a UDP Socket Server listening on port 80 (default)
    Created: 12-10-2016
    Author: Daniel R. Little
    Company: Little Squared, Inc.
    Description: This will script will start the ESP8266's WiFi and connect to the specified WiFi SSID/Password 
                 Once connected, if useStatic is true it will assin the static network variables to the interface.
    Defaults: Port: 80
              Output: JSON
]]--

local port = 80;
local output = "JSON";

local validRequests = "TH";

print("Opening TCP on Port "..port.." to accept sensor read requests.");
srv=net.createServer(net.TCP)
srv:listen(port,function(conn)
    conn:on("receive", function(client,request)
        print("Reqeust received:");
        print(request);
        local DHTBuffer = require("DHTBuffer");
        local data = '';
        if string.len(request) == 2 and request == "TH" then
            if output == "JSON" then
                data = DHTBuffer.genJSON();
            elseif out == "HTML", then
                data = DHTBuffer.getHTML();
            else
                print("Invalid output format specified.")
            end
        else
            if output == "JSON" then
                data = '{"status":-1, "message":"Unknown Request"}';
            elseif output == "HTML", then
                data = '<html><body><p>Unknown Request recevied:</p><p>'..request..'</p><p>Valid Request(s):'..validRequests..'</p></body></html>';
            else
                print("Invlid output format specified.");                           
            end
        end
        local result = nil;
        if output == "JSON" then 
            result = '{"chipID":"'..node.chipid()..'", "data":'..data..'}';
        else
            result = data;
        end
        print("Responding with: "..result);
        client:send(result);
        client:close();
        collectgarbage();
    end);
end);
