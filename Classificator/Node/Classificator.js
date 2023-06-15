var path = require("path");
var fs = require("fs");

/** Custom Error: RuntimeError */
function RuntimeError(message) {
  this.name = "RuntimeError";
  this.message = message || "Error running the current script";
  this.stack = new Error().stack;
}

var _f = function () {};
_f.prototype = Error.prototype;
RuntimeError.prototype = new _f();
RuntimeError.prototype.constructor = RuntimeError;
/** */

var findRuntimeDll = function () {
  var dlls = ["Classificator.dll", "Classificator.node", "md5.dll"];
  dlls.forEach(function (dll) {
    if (!fs.existsSync(dll)) {
      throw new RuntimeError("dependice DLL: " + dll + " not found");
    }
  });
};

findRuntimeDll();

var nativeBinding = require("./Classificator.node");

nativeBinding.LoadDllLibrary();

/**
 * Classificator progress teller function callback
 * @typedef {Function} Teller
 * @param {number} current - The current process file order
 * @param {number} total - Total files count
 * @param {boolean} isDup - The current processing file is a duplicated file
 * @param {string} filename - The current processing file's filename
 * @param {string} [newFilename] - If duplicated found, get the new filename
 * @returns {undefined}
 */

/**
 * Classificate is a function helps you classificate the duplicate files in target folder and move duplicated file to dest folder
 * @param {string} sourceFileDir - The source file folder
 * @param {string} [duplicatedFileDestDir] - (Optional)The duplicate files folder move to
 * @param {Teller} [teller] - (Optional)Progress teller function callback
 * @returns {string[]} - String array indicates which file duplicated
 */
function Classificate(sourceFileDir, duplicatedFileDestDir, teller) {
  duplicatedFileDestDir = duplicatedFileDestDir || null;

  var tellerAdjuest = function (current, total, isDup, filename, newFilename) {
    if (typeof teller == "function") {
      teller(current, total, isDup, filename, newFilename);
    }
  };

  return nativeBinding.Classificate(
    sourceFileDir,
    duplicatedFileDestDir,
    tellerAdjuest
  );
}

module.exports = {
  Classificate: Classificate,
};
