#include "ConfigServer.h"

#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "Config.h"
#include "Log.h"

const int server_port = 80;
ESP8266WebServer server(server_port);

const char* server_mode_ssid     = "weatherhour_config";
const char* server_mode_password = "weatherhour";

static void handleTemplate(const String& ssid, const String& pass, const String& lat, const String& lon, const String& info)
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
    html += "  <li> LAT : ";
    html += "    <input type='text' name='lat' value='" + lat + "' />";
    html += "  </li>";
    html += "  <li> LON : ";
    html += "    <input type='text' name='lon' value='" + lon + "' />";
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
    String lat;
    String lon;

    String info;
    if (Config::ReadWifiConfig(ssid, pass, lat, lon)) {
        info = "Successed to read wifi config.";
    } else {
        Log::Error("Failed to read wifi config.");
    }
    handleTemplate(ssid, pass, lat, lon, info);
}

static void handlePost()
{
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    String lat  = server.arg("lat");
    String lon  = server.arg("lon");

    String info;
    if (Config::WriteWifiConfig(ssid, pass, lat, lon) &&
        Config::ReadWifiConfig(ssid, pass, lat, lon)) {
        info = "Successed to write wifi config.";
    } else {
        info = "Failed to write wifi config.";
        Log::Error("Failed to write wifi config.");
    }
    handleTemplate(ssid, pass, lat, lon, info);
}

void ConfigServer::Start()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(server_mode_ssid, server_mode_password);
    String log = "WIFI_AP ssid=";
    log += server_mode_ssid;
    log += ", password=";
    log += server_mode_password;
    Log::Info(log.c_str());

    server.on("/", HTTP_GET,  handleGet);
    server.on("/", HTTP_POST, handlePost);
    server.begin();
    Log::Info("HTTP server started. ");
    while (true) {
        server.handleClient();
    }
}
