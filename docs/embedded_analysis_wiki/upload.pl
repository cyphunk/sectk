#!/usr/bin/perl
# 		==================================================================
#		**      Date: 2012-06-14					**
#            	** 	Author:- Nicholas:~ Wikipedia: [[en:User:Nichalp]]	**
# 		**	Filename:- upload.pl					**
#		** 	Licence:- Published under the GPLv3 licence		**
# 		**	Revision:- 1.3.2					**
#		**	Purpose: Read CSV file, format data as per wikisyntax,	**
#		**		 write Exif data, rotate or rename file, and 	**
# 		**		 then upload the file to Wikimedia commons. 	**
#		** 	URL: http://commons.wikimedia.org/wiki/User:Nichalp/Upload_script
#		==================================================================  
 
 
# 		==================================================================
#		**				Headers				**
#		==================================================================
 
use strict;
use warnings;
use utf8; 
use Data::Dumper; 
use LWP::Simple;
use LWP::UserAgent;
use HTTP::Request;
use HTTP::Response;
use HTTP::Cookies;
use Encode qw(encode);
use Image::ExifTool qw(:Public);
use Text::CSV_XS;
use Text::CSV::Encoded;
use Term::ReadKey;
use HTTP::Request::Common;
 
 
# 		==================================================================
#		**			Global declarations			**
#		==================================================================
 
#Exiftool
my $exifTool = new Image::ExifTool; 
 
#CSV Parser
my $csv;
if (defined $ARGV[3] && $ARGV[3] =~ /^UTF-?8$/i) {
 $csv = Text::CSV::Encoded->new({
    binary => 1,
    encoding => "utf8",
 });
} else {
 $csv = Text::CSV_XS->new({ binary => 1 });
}
my $file = 'upload.csv';
if (defined $ARGV[0] && $ARGV[0] ne "-") {
    $file = $ARGV[0];
}
my $secure;
if (defined $ARGV[1] && $ARGV[1] eq "secure") {
    $secure = 1;
}
if (defined $ARGV[2] && $ARGV[2] eq "disk") {
    $HTTP::Request::Common::DYNAMIC_FILE_UPLOAD = 1;
}
 
# Login information
my $ignore_login_error;
my @responses;
my @response;
my $response;
my $eckey;
my (@ns_headers, $browser);
 
 
# Initializing values to picked from CSV file
my ($current_name, $new_name, $description_language, $description, $date, $author, $permission_value, $category_1, $category_2, $category_3, $category_4, $category_5, $rotation, $coordinate, $latitude, $latitude_m, $latitude_s,$latitude_ref, $longitude, $longitude_m, $longitude_s, $longitude_ref, $geo_type, $geo_scale, $geo_region, $geo_heading, $geo_source, $altitude, $other_version1, $other_version2, $description_language_2, $description_2, $description_language_3, $description_3,  $embed_exif, $caption,  $country, $state, $place, $website, $keywords, $camera_info, $model, $aperture, $shutter, $film, $iso, $lens, $flickr, $flickr_url, $flickr_title, $flickr_photographer_url, $photographer_location, $other_information);
my ($year, $geo_parameters, $mastercategory, $photo_information, $flickr_self, $username, $password, $permission);
my @permission = ("user defined", "{{self|cc-by-sa-3.0,2.5,2.0,1.0|GFDL}}", "{{self|cc-by-sa-3.0}}", "{{self|cc-by-sa-3.0,2.5,2.0,1.0}}", "{{self|cc-by-sa-3.0|GFDL}}", "{{PD-self}}", "{{self|cc-pd}}", "{{self|cc-by-3.0}}"); # Storing permissions in an array
 
my $err=0; # Errors: Zero indicates no error. If "1", the script 'dies' and forces you to enter the required parameters.
my $geo_reference = ""; 
my ($gps_latitude_ref, $gps_longitude_ref, $licence, $altitude_ref, $gps_latitude, $gps_longitude);
my @exif_permission_information = ("user defined", "Released under the Creative Commons attribution and share-alike licences v1, 2, 2.5 and 3 and GFDL", "Released under Creative Commons attribution and share-alike licence v 3.0", "Released under the Creative Commons attribution and share-alike licences v1, 2, 2.5 and 3", "Released under Creative Commons licence attribution and share-alike v 3.0", "Released into Public Domain", "Released into public domain using the Creative Commons Public Domain Dedication", "Released under Creative Commons attribution licence v 3.0");
my ($exif_permission_text, $information, $full_description, $metadata);
 
 
# Storing geographic references
my %geolatlong= (
	N => "North",
	S => "South",
	E => "East",
	W => "West",
);
 
 
#Login information. You can directly store the $username variable if you wish by editing the line.
print "Enter your Commons username: ";
$username = <STDIN>;
print "Enter your Commons password: ";
if ($secure) {
 ReadMode('noecho');
 $password = ReadLine(0);
 ReadMode('normal');
} else {
 $password = <STDIN>;
}
chomp ($username, $password);
 
 
# 		==================================================================
#		**			End global declarations			**
#		==================================================================
 
 
 
#---------------------------------------------------------------------------------------------------------------
 
{# Begin integrity check block
 
# 		==================================================================
#		**			Part 1: Integrity check 		**
#		==================================================================
 
 
open(my $data, '<', $file) or die "Could not open '$file'\n";
<$data>;
while (my $line = <$data>) {
    chomp $line;
    if ($csv->parse($line)) {
	($current_name, $new_name, $description_language, $description, $date, $author, $permission_value, $category_1, $category_2, $category_3, $category_4, $category_5, $rotation, $coordinate, $latitude, $latitude_m, $latitude_s,$latitude_ref, $longitude, $longitude_m, $longitude_s, $longitude_ref, $geo_type, $geo_scale, $geo_region, $geo_heading, $geo_source, $altitude, $other_version1, $other_version2, $description_language_2, $description_2, $description_language_3, $description_3,  $embed_exif, $caption,  $country, $state, $place, $website, $keywords, $camera_info, $model, $aperture, $shutter, $film, $iso, $lens, $flickr, $flickr_url, $flickr_title, $flickr_photographer_url, $photographer_location, $other_information) = $csv->fields();       
 
# Converting to uppercase and lowercase
$latitude_ref = uppercase ($latitude_ref);
$longitude_ref = uppercase ($longitude_ref);
$geo_region = uppercase ($geo_region);
$flickr = uppercase ($flickr); 
$camera_info = uppercase ($camera_info);
$embed_exif = uppercase ($embed_exif);
$geo_type = lowercase ($geo_type);
$coordinate = lowercase ($coordinate);
 
#################################
 
# Check to see if compulsory fields "Description, Permissions, and Category" has been added. Warn and exit if not.
 
	if ($description eq ""){print "Please add a description to $current_name\n" ;$err=1;};
 
#Permissions
	if ($permission_value eq "")
		{print "Please enter the permission details to $current_name\n";$err=1}
	elsif (($permission_value > 7) || ($permission_value < 0) ) {print "Please enter a correct permissions value for $current_name\n"; $err=1;}
	elsif (($permission_value < 7) && ($permission_value >= 0) ) {}
	else {print "Invalid permissions for $current_name\n";$err=1;}
 
 
 
#Category check
	if ($category_1 eq ""){print "Please enter a category to $current_name\n"; $err=1;};
 
# Latitude and longitude cannot exist without each other. So checking. If both do not exist, ignore, as the template is optional.
 
	if (($latitude eq "") xor ($longitude eq ""))
		{
	 	 if ($latitude eq "") 
		 {
		   print "Please add the latitude to $current_name\n";
		  }; 
		if($longitude eq "")
		 {
		   print "Please add the longitude to $current_name\n";
		 };
		$err=1;
		};
 
# Checking dms values
	if ($coordinate eq "")
		{ $coordinate = 'd';}
	if (($coordinate eq 'd') || ($coordinate eq 'dms'))
		{}
	else	{
		print "Invalid coordinate system. Please enter either \'d\' or \'dms\' only\n";$err=1;
		}
 
#Checking for valid values if both are entered
	if (($latitude ne "") && ($latitude ne ""))
	{
		if($latitude_ref eq "") 
			{
			print "Please add the latitude reference (N or S) to $current_name\n";
			$err=1;
			}
 
		if($longitude_ref eq "")
			{
			print "Please add the longitude reference (E or W) to $current_name\n";$err=1}
 
		if ($latitude_ref ne "")
			{
			 if (($latitude_ref eq "N") || ($latitude_ref eq "S"))
				{
 
				}
			else 	
				{print "Invalid latitude reference\n"; $err=1;}
			}
		if ($longitude_ref ne "")
			{
			 if (($longitude_ref ne "E") || ($longitude_ref ne "W"))
				{ 
				}
 
				else 	
				{print "Invalid latitude reference\n"; $err=1;}			
			}
 
 
 
		if (($latitude < 0) || ($latitude > 90)) #latitude cannot exceed 90 degrees
		 		{
				print "$latitude is not a valid latitude\n"; $err=1;
		        	}
				if (($longitude < -180) || ($longitude > 180)) #Longitude cannot exceed 180 degrees
				{
				 print "$longitude is not a valid longitude\n";$err=1;	
				}
 
		if ($coordinate eq "dms")
			{
			if ($latitude_m ne "")
				{if (($latitude_m < 0) || ($latitude_m >= 60))
					{print "Invalid latitude minutes value for $current_name"; $err=1;}		
				}
			if ($latitude_s ne "")							
				{if (($latitude_s < 0) || ($latitude_s >= 60))
					{print "Invalid latitude seconds value for $current_name"; $err=1;}
				}
			if ($longitude_m ne "")					
				{if (($longitude_m < 0) || ($longitude_m >= 60))
				{print "Invalid longitude minutes value for $current_name"; $err=1;}		
				}
			if ($longitude_s ne "")	
				{if (($longitude_s < 0) || ($longitude_s >= 60))
					{print "Invalid longitude seconds value for $current_name";$err=1;
				}
			}
		}
	}
 
 
 
# Testing Flickr values. URL and Title fields are compulsory if Flickr values are set.
if ($flickr eq "Y")
  	{	
	if (($flickr_url eq "") || ($flickr_title eq ""))
		{
		if ($flickr_url eq "") {print "Please add the Flickr URL to $current_name: \n";$err=1;}
		if ($flickr_title eq "") {print "Please add the Flickr image title to $current_name: \n";$err=1;}
		}
	}
 
 
#Checking for valid geo_type parameters 
my $geo_type_err = 1; 
my @geo_type_params =  qw(country state adm1st adm2nd city airport airport isle waterbody forest river glacier edu pass railwaystation landmark); 
if ($geo_type ne "") 
	{ 
	for (0..$#geo_type_params) 
	 { 
	  if ($geo_type eq $geo_type_params[$_]) 
	   {$geo_type_err = 0} 
	 } 
	if ($geo_type_err == 1)  
	{print "Invalid \'type\' attributes for $current_name. Please enter a valid geotype.\n";$err=1;}
	} 
 
 
#Checking for geoheading issues
my @geo_heading_params = (0..360, "N", "NBE", "NNE", "NEBN", "NE", "NEBE", "ENE", "EBN", "E", "EBS", "ESE", "SEBE", "SE", "SEBS", "SSE", "SBE", "S", "SBW", "SSW", "SWBS", "SW", "SWBW", "WSW", "WBS", "W", "WBN", "WBW", "NWBW", "NW", "NWBN", "NNW", "NBW");
my $geo_heading_err = 1;
my $geo_heading_u = uppercase($geo_heading);
 
if ($geo_heading ne "")
	{
	for (0..$#geo_heading_params)
	 { 
	  if ($geo_heading_u eq $geo_heading_params[$_]) 
	   {$geo_heading_err = 0;} 
	 }
	 if ($geo_heading_err == 1)  
		{print "Invalid \'heading\' attribute for $current_name. Please enter a valid heading (0-360 or a compass point).\n";$err=1;}
	}
 
#Checking for valid Xform (JPEG transform) parameters
my $rotation_err =1;
my @rotation_values = (1..9, 90, 180, 270);
if ($rotation ne "")
	{
	for (0..$#rotation_values)
	 {
	  if ($rotation == $rotation_values[$_])
	   {$rotation_err = 0;}
	 }	 
if ($rotation_err == 1) 
	{print "Invalid transformation (Xform) attributes for $current_name. Please enter a valid number\n";$err=1;}
	}
 
 
} # End of if parser loop
 else {
        warn "Line could not be parsed: $line\n";
    }
 
} #End while loop
 
if ($err == 1) 
	{die "\nExiting! Please add the above values and try again...\n\n";};
 
# -----------------------------------------------------------------------------------------------------------------
 
# 		==================================================================
#		** 			Testing login				**
#		==================================================================
 
# 		==================================================================
#		** This part cannibalised from Eloquence's File Upload Script	**
#		==================================================================
 
 
$ignore_login_error=0;# should be obsolete due to wpSkipCookieCheck=1
 
$browser=LWP::UserAgent->new();
  @ns_headers = (
   'User-Agent' => 'Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.9.0.10) Gecko/2009042700 Firefox/3.0.10',
   'Accept' => 'image/jpeg, image/pjpeg, */*',
   'Accept-Charset' => 'iso-8859-1,*,utf-8',
   'Accept-Language' => 'en-US',
  );
 
$browser->cookie_jar({});
 
$response=$browser->post("http://commons.wikimedia.org/w/index.php?title=Special:UserLogin&action=submitlogin&type=login",
@ns_headers, Content_Type=>"application/x-www-form-urlencoded", Content_Encoding=>"utf-8", Content=>[wpName=>$username, wpPassword=>$password, wpRemember=>"1", wpSkipCookieCheck=>"1", wpLoginattempt=>"Log in"]);
 
 
# After logging in, we should be redirected to another page. 
# If we aren't, something is wrong.
#
if($response->code!=200 && $response->code!=302 && !$ignore_login_error) {
        print 
"Unable to login. This could be due to:
 
* The username ($username) or password might be incorrect.
  Solution: Edit upload.pl.
* You are trying to hack this script for other wikis. The wiki you
  are uploading to has cookie check disabled.
  Solution: Try setting \$ignore_login_error to 1 in upload.pl.
 
Writing the output from the server to debug.txt... ";
        open(DEBUG,">debug.txt") or die "Could not write file.\n";
        print DEBUG $response->as_string;
        print 
"Done!\n";
        close(DEBUG);
        exit 1;
}
 
 
# 		==================================================================
#		**			End integrity check	 		**
#		==================================================================
 
}# End integrity check block
 
 
 
 
 
 
 
 
#---------------------------------------------------------------------------------------------------------------
 
 
{# Begin formatting master block
# 		==================================================================
#		**			Part 2: Integration	 		**
#		==================================================================
 
open(my $data, '<', $file) or die "Could not open '$file'\n";
<$data>;
while (my $line = <$data>) {
    chomp $line;
    if ($csv->parse($line)) {
	($current_name, $new_name, $description_language, $description, $date, $author, $permission_value, $category_1, $category_2, $category_3, $category_4, $category_5, $rotation, $coordinate, $latitude, $latitude_m, $latitude_s,$latitude_ref, $longitude, $longitude_m, $longitude_s, $longitude_ref, $geo_type, $geo_scale, $geo_region, $geo_heading, $geo_source, $altitude, $other_version1, $other_version2, $description_language_2, $description_2, $description_language_3, $description_3,  $embed_exif, $caption,  $country, $state, $place, $website, $keywords, $camera_info, $model, $aperture, $shutter, $film, $iso, $lens, $flickr, $flickr_url, $flickr_title, $flickr_photographer_url, $photographer_location, $other_information) = $csv->fields();       
 
# Converting to uppercase and lowercase
$latitude_ref = uppercase ($latitude_ref);
$longitude_ref = uppercase ($longitude_ref);
$geo_region = uppercase ($geo_region);
$flickr = uppercase ($flickr); 
$camera_info = uppercase ($camera_info);
$embed_exif = uppercase ($embed_exif);
$geo_type = lowercase ($geo_type);
$coordinate = lowercase ($coordinate);
 
 
# Creating the gallery text file
open (GALLERY,">gallery.txt") or die "Could not write gallery file.\n";
print GALLERY "<gallery>\n";
 
$current_name=~s/\s/_/g; # converting spaces to underscores
 
# Calculating the altitude reference
if ($altitude ne '')
{
	if ($altitude < 0) 
		{ $altitude_ref = "Below Sea Level";}
		 else {$altitude_ref = "Above Sea Level";}
}
 
 
 
#################################
# This checks to see if the [[Template:Location]] geo-box is needed, and fills in the values
$geo_parameters = ""; #Clearing this value so that previous loop values are not appended.
if (($latitude ne "") && ($longitude ne ""))
	{
		if ($geo_type ne "")
		  {$geo_parameters.="type:$geo_type\_";};
		if ($geo_scale ne "")
		  {$geo_parameters.="scale:$geo_scale\_";};		
		if ($geo_region ne "")
		  {$geo_parameters.="region:$geo_region\_";};
		if ($geo_heading ne "")
		  {$geo_parameters.="heading:$geo_heading\_";};	
		if ($geo_source ne "")
		  {$geo_parameters.="source:$geo_source";};
		if ($geo_parameters ne "") 
			{		
			$geo_parameters=~s/^_*//; # Remove leading underscore
			$geo_parameters=~s/_*$//; # Remove trailing underscore
			}
		if ($coordinate eq 'dms')
		{
                 $latitude = abs ($latitude) ; $longitude = abs ($longitude);
	         $latitude = int ($latitude) ; $longitude = int ($longitude);
		 $geo_reference="{{Location|$latitude|$latitude_m|$latitude_s|$latitude_ref|$longitude|$longitude_m|$longitude_s|$longitude_ref|$geo_parameters}}\n";
		 $gps_latitude = "$latitude $latitude_m $latitude_s";
		 $gps_longitude = "$longitude $longitude_m $longitude_s";		
		}
 
		else 
			{
			if ($latitude_ref eq 'S')
			{$latitude = abs ($latitude); $latitude *=-1;
			} #Negative values for southern hemisphere
			if ($longitude_ref eq 'W')
			{
			$longitude = abs ($longitude); $longitude *=-1;
			} #Negative values for western hemisphere
			$geo_reference="{{Location dec|$latitude|$longitude|$geo_parameters}}\n";
			$gps_latitude = abs ($latitude) ; $gps_longitude = abs ($longitude);
			}
 
	#Expanding GPS References
	$gps_longitude_ref = "$geolatlong{$longitude_ref}";
	$gps_latitude_ref = "$geolatlong{$latitude_ref}";
 
	}
 
 
 
 
# Extracting the date
$date =  unpack ("A10",$date);  
$date=~tr/:/-/d;
$year = unpack ("A4", $date);
 
#################################
#Permissions
	if ($permission_value == 0)	
	 {
	defaultlicence:	 
	 print "Enter custom permission: ";
	  $licence = <STDIN>;
	  chomp $licence;
	  if ($licence =~ /^([1-7])$/) {
	   $permission_value = $1;
	   $licence = $permission[$permission_value];
	  }
	}
	 elsif (($permission_value >= 1) && ($permission_value <= 7))
	  {
	   $licence = $permission[$permission_value];
	   }
	else {print "Invalid licence\n"; goto defaultlicence;}
 
$exif_permission_text = $exif_permission_information[$permission_value];
 
 
 
 
# 		==================================================================
#		**		Part 3: Formatting metadata elements		**
#		==================================================================
 
##################################
# Rotation -- Adds the {{rotate}} template to the article if the Xform field is set to 90, 180 or 270. Rotatebot will automatically rotate the image post upload at 18:00 hrs UTC.
my $rot = "" ;
	if ($rotation ne "")
	 {
		if (($rotation == 90) || ($rotation == 180) || ($rotation == 270))
	 {$rot = "{{rotate|$rotation}}\n";} 
		 }
 
print "$shutter\n";
#Adding shutter values
if ($shutter ne "")
{
my $info = $exifTool->ImageInfo($current_name);  
	$shutter = $$info{ShutterSpeedValue}; #regaining shutter values since spreadsheets autoformat them.
}
 
print "$shutter\n";
 
#Author
my $computed_author;
if ($author eq "")
{
$computed_author = $username;
}
else
{
$computed_author = "$author ([[User:$username|$username]])";
}
 
 
##################################
# Camera information: [[Template:Photo information]]
	if ($camera_info eq "Y")
	  {
	  $aperture=~s/^'*//; # Remove leading single quote	
	   $photo_information="{{Photo Information\n| Model\t\t= $model\n| Aperture\t= $aperture\n| Shutter\t= $shutter\n| Film\t\t= $film\n| ISO\t\t= $iso\n| Lens\t\t= $lens\n}}\n";
	 };	
 
##################################
#Flickr information: [[Template:Flickr-self]]
 
	if ($flickr eq "Y")
	  {
	   if ($flickr_url ne "")
		{
		 $flickr_self="{{Flickr-self\n| Description\t\t= $description\n| Flickr_url\t\t= $flickr_url\n| Title\t\t\t= $flickr_title\n| Taken\t\t\t= $date\n| Photographer_url\t= $flickr_photographer_url\n| Photographer\t\t= $computed_author\n| Photographer_location\t= $photographer_location\n| Reviewer\t\t= \n| Permission\t\t=\n}}";
		}
	  else { print "Please enter the Flickr URL\n"; $err=1;}
	}
 
 
#Formatting the Description parameter
 
if ($description_language eq "")
	{$description_language = "en";}; # Default language set to English;
 
$full_description = "\n{{$description_language|$description}}";
if (($description_language_2 ne "") && ($description_2 ne ""))
	{
	$full_description ="\n{{$description_language_2|$description_2}}";
	}
if (($description_language_3 ne "") && ($description_3 ne ""))
	{
	$full_description .="\n{{$description_language_3|$description_3}}";
	}
 
 
# Categories: Combining all categories as one master category
	$mastercategory="[[Category:$category_1]]";
	if ($category_2 ne "")
	 {$mastercategory.="\n[[Category:$category_2]]" ;}
	if ($category_3 ne "")
	 {$mastercategory.="\n[[Category:$category_3]]" ;}
	if ($category_4 ne "")
	 {$mastercategory.="\n[[Category:$category_4]]" ;}
	if ($category_5 ne "")
	 {$mastercategory.="\n[[Category:$category_5]]" ;}
	$mastercategory.="\n[[Category:Files uploaded by Nichalp's script]]\n\n";
 
 
##################################
#Other versions
my $other_versions = "";
	if (($other_version1 ne "") || ($other_version2 ne ""))
	  {
		if ($other_version1 ne "")
		{
		$other_version1 = "\n* [[:Image:$other_version1|$other_version1]]";
	  	}
		if ($other_version2 ne "")
		{
		$other_version2 = "\n* [[:Image:$other_version1|$other_version1]]";
	  	}
	$other_versions = "$other_version1.$other_version2";
	}
 
 
 
# Information template
 
$information="\n== Summary ==\n\n{{Information\n| Description\t= $full_description\n| Source\t= Own work by uploader\n| Author\t= $computed_author\n| Date\t\t= $date\n| Permission\t= \n| Other versions= $other_versions\n}}\n";
 
 
# Begin optional template integration
$metadata = $information;
	if (defined $geo_reference)
	{
	  $metadata .= "$geo_reference\n";
	};
	if (defined $photo_information)
	 {
	  $metadata .= "$photo_information\n";
	 };
	if (defined $flickr_self)
	{
	  $metadata .= "$flickr_self\n";
	};
	if ($rot ne "")
	{
	  $metadata .= "$rot\n";
	};	
	if (defined $other_information)
	{
	  $metadata .= "\n$other_information\n";
	};
 
# Integrating remaining templates into one 
 
$metadata .="\n== [[Commons:Copyright tags|Licensing]] ==\n$licence\n\n\n$mastercategory\n<!-- Uploaded by Nichalp's upload script. Contact [[User:Nichalp|Nichalp]] for more information. -->\n";
 
 
#-----------------------------------------------------------------------------------------------------------
 
 
# 		==================================================================
#		**		Part 4: Renaming and writing Exif data		**
#		==================================================================
 
##############################################
#Image manipulations (rotations, transformations etc
my $command;
my %rot_args = ( 
	2 => '-flip horizontal', 
	3 => '-rotate 180', 
	4 => '-flip vertical', 
	5 => '-transpose', 
	6 => '-rotate 90', 
	7 => '-transverse', 
	8 => '-rotate 270',
	9 => '-greyscale', 
  		); #jpegtran parameters
 
if ($rotation ne "")
{
if (($rotation != 90) || ($rotation !=180) || ($rotation != 270))
	{
	if (($rotation > 1) && ($rotation < 10))
	{
	$command = "jpegtran -copy all $rot_args{$rotation} $current_name > imgtemp.jpg"; #New modified file created
	system ($command); # Executes the jpegtran command
	$exifTool->SetNewValue(Orientation => "Horizontal (normal)"); # Makes sure the Exif Orientation value is written 
	$exifTool->WriteInfo("imgtemp.jpg"); # Writing the value into the temporary image
	unlink ("$current_name"); #Deleting the old file
	rename ("imgtemp.jpg", "$current_name"); # Renaming the temporary file
	unlink ("imgtemp.jpg"); # Deleting the temporary file
	}
	}
}
 
###############################################
print "-" x 75 . "\n";
# Renaming
 
if ($new_name ne "")
	{
	$new_name=~s/\s/_/g; # converting spaces to underscores
	$new_name .= ".jpg" unless $new_name =~ /\.jpe?g$/i; # adding JPG extn	
	print "Renaming $current_name to $new_name... ";
	rename ($current_name, $new_name);
	$current_name = $new_name;
	$new_name="";
	print "Successful!\n";	
	}
 
 
#Begin Exif addition
if ($embed_exif ne "N")
 
{
print "Embedding Exif information to $current_name... ";
if ($author eq '')
{
$author = $username;
}
#Author
$exifTool->SetNewValue(URL => "$website");
$exifTool->SetNewValue(WebStatement => "$website");
$exifTool->SetNewValue(Author => "$author");
$exifTool->SetNewValue(Creator => "$author");
$exifTool->SetNewValue(Artist => "$author");
$exifTool->SetNewValue(Credit => "$author");
$exifTool->SetNewValue(OwnerName => "$author");
 
 
#Copyrights
	if (($permission_value == 1) || ($permission_value == 2) || ($permission_value == 3) || ($permission_value == 4 ) 			|| ($permission_value == 7)) #PD images do not require copyright attribution
	{
	$exifTool->SetNewValue(Copyright => "© Copyright $year $author");
	$exifTool->SetNewValue(Rights => "© Copyright $year $author. $exif_permission_text");
	$exifTool->SetNewValue(CopyrightNotice => "$author");
	}
	elsif  (($permission_value == 5) || ($permission_value == 6))
		{
			$exifTool->SetNewValue(Rights => "Released into Public Domain by $author");
		}
 
#Locations
$exifTool->SetNewValue(Country => "$country");
$exifTool->SetNewValue(State => "$state");
$exifTool->SetNewValue(Location => "$place");
$exifTool->SetNewValue(City => "$place");
 
#GPS information
	if ($altitude ne '')
		{
		$exifTool->SetNewValue(GPSAltitude => "$altitude");
		$exifTool->SetNewValue(GPSAltitudeRef => "$altitude_ref");
		}
	if ($latitude ne '') # Since we have already tested, only one paramter is necessary
		{
		$exifTool->SetNewValue(GPSLongitude => "$gps_longitude");
		$exifTool->SetNewValue(GPSLongitudeRef => "$gps_longitude_ref");
		$exifTool->SetNewValue(GPSLatitude => "$gps_latitude");
		$exifTool->SetNewValue(GPSLatitudeRef => "$gps_latitude_ref");
		}
 
#Caption
$exifTool->SetNewValue(Title => "$caption");
$exifTool->SetNewValue(Headline => "$caption");
$exifTool->SetNewValue(ObjectName => "$caption");
$exifTool->SetNewValue(ImageDescription => "$caption");
 
#Other fields
$exifTool->SetNewValue(Category => "$category_1");
$exifTool->SetNewValue(SupplementalCategories => "$category_2");
$exifTool->SetNewValue(Keywords => "$keywords");
 
#Writing
$exifTool->WriteInfo("$current_name");
print "Successful!\n";
 
}
 
#-----------------------------------------------------------------------------------------------------------
 
 
# 		==================================================================
#		**		Part 5: Uploading images to commons		**
#		==================================================================
 
 
print "Uploading $current_name to the Wikimedia Commons. \nDescription: ";      
        print $description."\n";
        uploadfile:
	$eckey=encode('utf8',$metadata);
        if($eckey ne $metadata) {
                symlink("$metadata","$eckey");
        }
	$response=$browser->post("http://commons.wikimedia.org/wiki/Special:Upload",
        @ns_headers,Content_Type=>'form-data',Content=>
        [
                wpUploadFile=>["$current_name"],
                wpDestFile=>["$current_name"],
                wpUploadDescription=>encode('utf8',$metadata),
                wpUploadAffirm=>"1",
                wpIgnoreWarning=>"1",
                wpUpload=>"Upload file"
        ]);#~wpUploadFile=>$path?, wpWatchthis=>"1"?
push @responses,$response->as_string;
        if($response->code!=200 && $response->code!=302) {
                print "Upload failed! Will try again. Output was:\n";
                print $response->as_string;
                goto uploadfile;
        } else {
                print "Uploaded successfully.\n";
        }               
 
 
#-----------------------------------------------------------------------------------------------------------
 
# 		==================================================================
#		**			Part 5: Gallery 			**
#		==================================================================
 
 
#Just incase you need a gallery
print GALLERY "Image:$current_name|$description\n";
 
 
} # End of if loop
 else {
        warn "Line could not be parsed: $line\n";
    }
 
} # End of while loop
 
#Log file
print "All OK. Log written to debug.txt.\n";
open(DEBUG,">debug.txt") or die "Could not write file.\n";
print DEBUG @responses;
       close(DEBUG);
 
#Closing the gallery
print GALLERY '</gallery>';
       close(GALLERY);
 
print "\n ~*~ Thank you for using Nichalp's script! ~*~\n";
 
} #End formatting master block
 
 
 
#-----------------------------------------------------------------------------------------------------------
 
 
{ # Begin subroutine block
# 		==================================================================
#		**			Subroutine block			**
#		==================================================================
 
# Convert to uppercase
sub uppercase 	
{
my $uppercase = $_[0] ;
if (defined $uppercase) 
	{
	$uppercase =~ tr/a-z/A-Z/;
	return $uppercase;
	}
else {return $uppercase;}
}
 
#Convert to lowercase
sub lowercase 	
{
my $lowercase = $_[0] ;
if (defined $lowercase)
	{
	$lowercase =~ tr/A-Z/a-z/;
	return $lowercase;
	}
else {return $lowercase;}
}
 
# 		==================================================================
#		**			Subroutine block ends			**
#		==================================================================
}# End subroutine block
 
 
 
# -----------------------------------------------------------------------------------------------------
 
__END__
 
 
 
Credits:
Wish to thank the following: 
1. User Eloquence (Erik Möller) File upload service script --> http://commons.wikimedia.org/wiki/Commons:File_upload_service -- Mercilessly hacked. :)
2. Phil Harvey's ExifTool --> http://www.sno.phy.queensu.ca/~phil/exiftool/
