/**
 * Guardian RX — Google Sheets Webhook
 * Appends a row based on JSON POST body.
 * Expected keys:
 *  ts_iso, deviceId, container, event, med_name, dosage, due_hhmm, result, rssi
 */
function doPost(e) {
  try {
    var body = JSON.parse(e.postData.contents);
    var ss = SpreadsheetApp.getActiveSpreadsheet();
    var sh = ss.getSheets()[0];

    var row = [
      body.ts_iso || new Date().toISOString(),
      body.deviceId || "",
      body.container || "",
      body.event || "",
      body.med_name || "",
      body.dosage || "",
      body.due_hhmm || "",
      body.result || "",
      body.rssi || ""
    ];

    sh.appendRow(row);
    return ContentService.createTextOutput("OK").setMimeType(ContentService.MimeType.TEXT);
  } catch (err) {
    return ContentService.createTextOutput("ERR: " + err).setMimeType(ContentService.MimeType.TEXT);
  }
}
