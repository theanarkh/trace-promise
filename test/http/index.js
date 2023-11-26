const http = require('http');
const { writeFileSync } = require('fs');
const path = require('path');
const { enable, disable, tree } = require('../../lib');

enable();

const server = http.createServer(async (req, res) => {
    await new Promise((resolve) => setTimeout(resolve, 5000));
    res.end();
}).listen(9999, () => {
    http.get('http://localhost:9999', () => server.close());
});

process.on('exit', () => {
    writeFileSync(path.resolve(__dirname, 'trace.json'), JSON.stringify(tree, null, 4));
    disable();
});