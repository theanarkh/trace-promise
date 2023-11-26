
const { enable, disable, tree } = require('../../lib');
const { writeFileSync } = require('fs');
const path = require('path');

enable();

async function run() {
  await new Promise((resolve) => {
    setTimeout(resolve, 1000);
  });
  await new Promise((resolve) => {
    setTimeout(resolve, 2000);
  });
}
  
run();
  
process.on('exit', () => {
  writeFileSync(path.resolve(__dirname, 'trace.json'), JSON.stringify(tree, null, 4));
  disable();
});