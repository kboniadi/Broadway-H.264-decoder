function download() {
  // Read the file
  var fileContents = FS.readFile('/decode.yuv');

  // Create a blob with the file contents
  var blob = new Blob([fileContents], {type: 'application/octet-stream'});

  // Create a download link
  var downloadLink = document.createElement('a');
  downloadLink.href = URL.createObjectURL(blob);
  downloadLink.download = 'decode.yuv';

  // Click the download link to trigger the download
  downloadLink.click();
}
window.download = download