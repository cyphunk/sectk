#! /usr/bin/perl
 
# 		==================================================================
#		**      Date: 2012-06-14					**
#            	** 	Author:- Nicholas:~ Wikipedia: [[en:User:Nichalp]]	**
# 		**	Filename:- csv_creator.pl				**
#		** 	Licence:- Published under the GPLv3 licence		**
# 		**	Revision:- 1.0.2					**
#		**	Purpose: Scan all .JPG files in a directory, then read  **
#		**		 corresponding Exif information, and then list 	**
#		**		 them in a CSV file				**
#		** 	URL: http://commons.wikimedia.org/wiki/User:Nichalp/Upload_script
#		==================================================================  
 
 
use strict;
use warnings;
use utf8;
use Image::ExifTool qw(:Public);
 
#Declaring variables
my $exifTool = new Image::ExifTool;
my $file;
my $info;
my @files;
my ($author, $date, $aperture, $shutter, $iso, $camera_model, $lens);
 
 
# Creating the CSV file
open (UPLOAD,">upload.csv") or die "Could not write file.\n";
 
# Opening the directory
opendir(DIR, ".");
 
# Selecting only JPG file types
@files = grep (/\.jpe?g$/i, readdir(DIR));
closedir(DIR);
 
# Printing headers
print UPLOAD '"Current name","New name","Description lang","Description","Date","Author name","Permissions","Category1","Category2","Category3","Category4","Category5","Xform","Coordinate type","Lat deg","Lat min","Lat sec","Lat Ref","Long deg","Long min","Long sec","Long Ref","Type","Scale","Region","Heading","Source","Altitude","Other versions 1","Other versions 2","Description lang 2","Description 2","Description lang 3","Description 3","Embed exif?","Caption","Country","State","Place","Website","Keywords","Camera info?","Camera Model","Aperture","Shutter","Film","ISO","Lens","Flickr?","Flickr URL","Flickr image title","Flickr photographer URL","Flickr photographer location","Other information"',"\n";
 
foreach $file (@files) {
	$info = $exifTool->ImageInfo($file); 
	my $rotation = $exifTool->GetValue('Orientation', 'ValueConv'); 
	$info = $exifTool->ImageInfo($file);    
	$date = $$info{DateTimeOriginal};
	$camera_model = $$info{Model};
	$aperture = $$info{ApertureValue};
	$iso = $$info{ISO};
	$lens = $$info{Lens};
	$shutter = $$info{ShutterSpeedValue};
#printing values	
	print UPLOAD "\"$file\"";
	print UPLOAD ',,,,';
	if (defined $date) #Checking for null values
	 {print UPLOAD "\"$date\"";} 
	 else {print UPLOAD ',';};
	print UPLOAD ',,,,,,,,';
	if (defined $rotation)
	 {print UPLOAD "\"$rotation\"";} 
	 else {print UPLOAD ',';};
	print UPLOAD ',,,,,,,,,,,,,,,,,,,,,,,,,,,,,,';
	if (defined $camera_model)
	 {print UPLOAD "\"$camera_model\"";} 
	 else {print UPLOAD ',';};
	print UPLOAD ',';
	if (defined $aperture)
	 {print UPLOAD "\"$aperture\""} 
	 else {print UPLOAD ',';};	
	print UPLOAD ',';
	if (defined $shutter)
	 {print UPLOAD "\"'$shutter\"";} #The leading single quote is to prevent spreadsheets from auto-formatting as a date. We will remove it later.	
	 else {print UPLOAD ',';};
	print UPLOAD ','; 
	print UPLOAD "Digital".',';
	if (defined $iso)
	 {print UPLOAD "\"$iso\"".','} 
	 else {print UPLOAD ',';};
	if (defined $lens)
	 {print UPLOAD "\"$lens\""} 
	 else {print UPLOAD ',';};
	print UPLOAD ',,,,,,',"\n";
	}
print "upload.csv successfully created!\n";
close (UPLOAD);
 
__END__
