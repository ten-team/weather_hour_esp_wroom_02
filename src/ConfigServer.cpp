#include "ConfigServer.h"

#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "Config.h"

ESP8266WebServer server(80);

const char* server_mode_ssid     = "yudetamago_config";
const char* server_mode_password = "yudetamago";

static void handleGet()
{
    String ssid;
    String pass;
    bool result = Config::ReadWifiConfig(ssid, pass);

    String html = "<h1>WiFi config</h1>";
    if (result) {
        html += "Successed to read wifi config.";
    } else {
        Serial.println("Failed to read wifi config.");
    }
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

static void handlePost()
{
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");

    String html = "<h1>WiFi config</h1>";
    if (Config::WriteWifiConfig(ssid, pass) &&
        Config::ReadWifiConfig(ssid, pass)) {
        html += "Successed to write wifi config.";
    } else {
        html += "Failed to write wifi config.";
        Serial.println("Failed to write wifi config.");
    }
    html += "<ul>";
    html += "  <li>SSID : " + ssid + "</li>";
    html += "  <li>PASS : " + pass + "</li>";
    html += "</ul>";
    server.send(200, "text/html", html);
}

void ConfigServer::Start()
{
    WiFi.mode(WIFI_AP);
    WiFi.softAP(server_mode_ssid, server_mode_password);

    server.on("/", HTTP_GET,  handleGet);
    server.on("/", HTTP_POST, handlePost);
    server.begin();
    Serial.println("HTTP server started.");
    while (true) {
        server.handleClient();
    }
}
