Import-Module .\Classificator.psm1

function Tell-Progress {
    param(
        [int]$current,
        [int]$total,
        [bool]$isDuplicated,
        [string]$filename,
        [string]$newFilename
    )

    $precent = $current / $total * 100
    
    $duplicatedFoundInfo = ""
    if ($isDuplicated) {
        $duplicatedFoundInfo = ", Duplicated found, move to $newFilename"
    }

    Write-Host "($precent)% Current procress files: $filename $duplicatedFoundInfo"
}

$srcDir = "../SampleFileGen/GenPics"
$dstDir = "../rst"

$duplicatedFiles = Classificate-File $srcDir $dstDir ${function:Tell-Progress}

Write-Host "Duplicated Files moved to $dstDir : "
Write-Host $duplicatedFiles