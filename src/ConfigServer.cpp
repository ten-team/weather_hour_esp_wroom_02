#include "ConfigServer.h"

#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "Config.h"
#include "Log.h"

ESP8266WebServer server(80);

const char* server_mode_ssid     = "yudetamago_config";
const char* server_mode_password = "yudetamago";

static void handleTemplate(const String& ssid, const String& pass, const String& info)
{
    String html = "<h1>WiFi config</h1>";
    html += info;
    html += "<form method='post'>";
    html += "<ul>";
    html += "  <li> SSID : ";
    html += "    <input type='text' name='ssid' value='" + ssid + "' />";
    html += "  </li>";
    html += "  <li> PASS : ";
    html += "    <input type='text' name='pass' value='" + pass + "' />";
    html += "  </li>";
    html += "</ul>";
    html += "<input type='submit' />";
    html += "</form>";
    server.send(200, "text/html", html);
}

static void handleGet()
{
    String ssid;
    String pass;

    String info;
    if (Config::ReadWifiConfig(ssid, pass)) {
        info = "Successed to read wifi config.";
    } else {
        Log::Error("Failed to read wifi config.");
    }
    handleTemplate(ssid, pass, info);
}

static void handlePost()
{
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");

    String info;
    if (Config::WriteWifiConfig(ssid, pass) &&
        Config::ReadWifiConfig(ssid, pass)) {
        info = "Successed to write wifi config.";
    } else {
        info = "Failed to write wifi config.";
        Log::Error("Failed to write wifi config.");
    }
    handleTemplate(ssid, pass, info);
}

void ConfigServer::Start()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(server_mode_ssid, server_mode_password);

    server.on("/", HTTP_GET,  handleGet);
    server.on("/", HTTP_POST, handlePost);
    server.begin();
    Log::Info("HTTP server started.");
    while (true) {
        server.handleClient();
    }
}
