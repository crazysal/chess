=pod   # Documentation comment

File "unpackIR.pl", by KWR.
Unpack IRall1file.cpp into separate .h and .cpp files.

=cut

open(DAT,"IRall1file.cpp") || die("Could not open file!");
@codeLines = <DAT>;
$numLines = @codeLines;

$fileDivPat = "^\/\/File(-*)---([^-].*[^-])---";

# $fileName = "/dev/null";
# open(WRI,">$fileName");

$i = 0;
$foundStart = false;
while ($i < $numLines) {
   if ($codeLines[$i] =~ m/$fileDivPat/) {
      if ($foundStart) {
         close(WRI);
      } else {
         $foundStart = true;   # and thereafter
      }
      $fileName = $2;
      open(WRI,">$fileName") || die("Could not open " . $fileName);
   }
   if ($foundStart) {
      print WRI $codeLines[$i];
   }
   $i++;
}

close(WRI);
close(DAT);

