#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>
//Generate live browser UI w/ curtain state and scheduled times
inline String getIndexPage(bool isOpen, int openH, int openM, int closeH, int closeM) {
  // pad single digits with leading zeros for proper time formatting
  String openHStr  = (openH < 10 ? "0" : "") + String(openH);
  String openMStr  = (openM < 10 ? "0" : "") + String(openM);
  String closeHStr = (closeH < 10 ? "0" : "") + String(closeH);
  String closeMStr = (closeM < 10 ? "0" : "") + String(closeM);

  // Return the HTML page as a raw string literal
  return R"rawliteral(<!DOCTYPE html>
<html>
<head>
  <title>Curtain Controller</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; margin: 20px; background: #f5f5f7; }
    .card { background: white; padding: 20px; border-radius: 12px; max-width: 600px; margin: 0 auto 20px auto; box-shadow: 0 4px 6px rgba(0,0,0,0.07); }
    h2, h3 { margin-top: 0; color: #1d1d1f; }
    input[type="time"] { font-size: 16px; padding: 8px; border: 1px solid #ccc; border-radius: 6px; width: 100%; box-sizing: border-box; }
    button { background: #0071e3; color: white; border: none; font-size: 16px; padding: 10px 20px; border-radius: 8px; cursor: pointer; margin-top: 10px; width: 100%; font-weight: bold; }
    #console { background: #1e1e1e; color: #4af626; font-family: monospace; font-size: 13px; padding: 15px; border-radius: 8px; height: 260px; overflow-y: scroll; white-space: pre-wrap; word-break: break-all; }
  </style>
</head>
<body>
  <div class="card">
    <h2>Smart Curtain Control</h2>
    <p>Current State: <b>)rawliteral" + String(isOpen ? "OPEN" : "CLOSED") + R"rawliteral(</b></p>
    <form action="/save" method="GET">
      <label><b>Open Time:</b></label><br>
      <input type="time" name="open" value=")rawliteral" + openHStr + ":" + openMStr + R"rawliteral("><br><br>
      <label><b>Close Time:</b></label><br>
      <input type="time" name="close" value=")rawliteral" + closeHStr + ":" + closeMStr + R"rawliteral("><br><br>
      <button type="submit">Save Schedule</button>
    </form>
  </div>
  <div class="card">
    <h3>Wireless Live Terminal</h3>
    <div id="console">Loading logs...</div>
  </div>
  <script>
    function fetchLogs() {
      fetch('/logs')
        .then(response => response.text())
        .then(data => {
          const consoleEl = document.getElementById('console');
          const isScrolledToBottom = consoleEl.scrollHeight - consoleEl.clientHeight <= consoleEl.scrollTop + 20;
          consoleEl.innerText = data;
          if (isScrolledToBottom) {
            consoleEl.scrollTop = consoleEl.scrollHeight;
          }
        });
    }
    setInterval(fetchLogs, 1500);
    fetchLogs();
  </script>
</body>
</html>)rawliteral";
}

#endif // WEB_PAGE_H