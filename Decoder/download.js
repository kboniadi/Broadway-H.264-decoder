function download() {
  // Read the file
  let fileContents = FS.readFile('/decode.yuv');

  // Create a blob with the file contents
  let blob = new Blob([fileContents], {type: 'application/octet-stream'});

  // Create a download link
  let downloadLink = document.createElement('a');
  downloadLink.href = URL.createObjectURL(blob);
  downloadLink.download = 'decode.yuv';

  // Click the download link to trigger the download
  downloadLink.click();
}
window.download = download