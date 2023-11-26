'use strict';
const { setCallback } = require(`../build/Release/binding`);

const tree = {
  child: {}
};

function findNodeById(id, node = tree) {
  if (node.id === id) {
    return node;
  } else {
    for (const [_, v] of Object.entries(node.child)) {
      const result = findNodeById(id, v);
      if (result) {
        return result;
      }
    }
  } 
};

function init(obj) {
  if (obj.pid) {
    const result = findNodeById(obj.pid);
    result.child[obj.id] = {
      ...obj,
      child: {},
      start: Date.now()
    };
  } else {
    tree.child[obj.id] = {
      ...obj,
      child: {},
      start: Date.now()
    }
  }
}

function resolve(obj) {
  const result = findNodeById(obj.id);
  result.end = Date.now();
  result.cost = result.end - result.start;
}

function safaCallWapper(fn) {
  return function() {
    try {
      fn.apply(this, arguments);
    } catch (e) {
      console.error(e);
    }
  }
}

module.exports = {
  tree,
  enable: () => setCallback(safaCallWapper(init), safaCallWapper(resolve)),
  disable: () => setCallback(),
};
