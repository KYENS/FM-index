#!/usr/bin/perl
use strict;

my $host_file = 'host_query.txt';
my $ans_file = 'mstep_result.txt';
open(F_HOST, '<', $host_file) or die $!;
open(F_2STEP, '<', $ans_file) or die $!;

my $COUNT=0;
while(<F_2STEP>){
	my @str=split ':', $_ ;
	my @str_2=split ' ', $str[1] ;
	#print $_;
	#print $str[0], $str_2[0], $str_2[1] ,"\n";
	my $temp=$str[0].':';
	#print $temp,"\n";
	while(my $H_IN=<F_HOST>){
		my @sub_str=split ':', $H_IN ;
		#print $H_IN, $sub_str[0];
		my @sub_str_2=split ' ', $sub_str[1] ;
		my $reg=$sub_str[0].':';
		#print $temp,' ',$reg,"\n";
		if($temp eq $reg){
			#print "PATTERN $COUNT \n";
			if($str_2[0]!=$sub_str_2[0]  or $str_2[1] != $sub_str_2[1]){
				print $reg, "QUERY $COUNT ERROR!!!\n";
				print $str_2[0],' ',$str_2[1] ,' ',$sub_str_2[0],' ', $sub_str_2[1] ,"\n";
			}
			else {
				print "PATTERN $COUNT PASS \n";
			}
		}
	}
	$COUNT++;
	seek F_HOST, 0, 0;

	#print "count\n";

}
#print "Hello, world!\n";
