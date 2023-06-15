var Classificator = require("./Classificator");

var duplicated = Classificator.Classificate(
  "../SampleFileGen/GenPics",
  "../rst",
  function (current, total, isDup, filename, newFilename) {
    var precent = (current / total) * 100;

    var duplicateInfo = "";
    if (isDup) {
      duplicateInfo = " , Duplicate found, new filename is: " + newFilename;
    }

    console.log("(" + precent + "%) File: " + filename + duplicateInfo);
  }
);

console.log("Duplicated files: ");
console.log(duplicated);
