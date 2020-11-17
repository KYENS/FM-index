#!/usr/bin/perl
use strict;
my $L_num='Num_L_in_chip.txt';
open(L_num_input, '<', $L_num) or die $!;
my $num_of_Lterm_in_chip= <L_num_input>;
close L_num_input;
#my $num_of_Lterm_in_chip='894,875,713,710,718,743,737,714,757,748,727,835,716,745,724,704,700,797,718,770,708,911,814,713,728,767,696,821,734,746,760,721,713,834,760,724,746,726,797,728,724,748,707,901,882,720,744,717,744,806,755,715,712,695,813,688,700,717,784,808,733,713,724,744,703,755,737,753,724,721,782,712,743,727,808,905,804,745,735,727,677,785,712,746,699,692,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0';
my @arr_c=split(',',$num_of_Lterm_in_chip);
my @rows_to_open=(1,1,1,2,13,76,375,3365);
my @cols_to_open=(1,2,15,-1,-1,-1,-1,-1);
my $row_num=131072*16;
my @unit_num=($row_num/$rows_to_open[0],$row_num/$rows_to_open[1],$row_num/$rows_to_open[2],$row_num/$rows_to_open[3],$row_num/$rows_to_open[4],
				$row_num/$rows_to_open[5],$row_num/$rows_to_open[6],$row_num/$rows_to_open[7]);
my @unit_per_bank=($unit_num[0]/16,$unit_num[1]/16,$unit_num[2]/16,$unit_num[3]/16,$unit_num[4]/16,$unit_num[5]/16,$unit_num[6]/16,$unit_num[7]/16);

my $F_step=$ARGV[0];
my $m_step=$ARGV[1];
my $chip_id=$ARGV[2];
my $host_file = 'traces/'.$chip_id.'.txt';
print  "$num_of_Lterm_in_chip\n$host_file\n";
open(F__INPUT, '<', $host_file) or die $!;

print  "$m_step\n";

my $file_name = $F_step.'_'.$m_step.'_term_row_access.txt';
open(F_OUTPUT, '>', $file_name) or die $!;

while(<F__INPUT>){
	my @arr=split(',',$_);
	my @num;
	my $div=0;
	my @CHIP= split(':',$arr[0]);
	my @MIN= split(':',$arr[1]);
	my @MAX= split(':',$arr[2]);
	my @LINT= split(':',$arr[3]);
	my @MIN_s= split('\n',$MIN[1]);
	my @MAX_s= split('\n',$MAX[1]);
	my @LINT_s= split('\n',$LINT[1]);
	if($arr_c[int($CHIP[1])]==0){
		$div=1;
	}else{
		$div=$arr_c[int($CHIP[1])];
	}
	if($MIN_s[0]<0){
		$MIN_s[0]=0;
	}
	my $min_unit_id=int($unit_num[$m_step-1]*$MIN_s[0]/$div);
	my $max_unit_id=int($unit_num[$m_step-1]*$MAX_s[0]/$div);
	my $min_row_id=($min_unit_id%$unit_per_bank[$m_step-1])*$rows_to_open[$m_step-1],
	my $max_row_id=($max_unit_id%$unit_per_bank[$m_step-1])*$rows_to_open[$m_step-1];
	
	if($m_step<4){
		my $min_bk_id=int(($min_unit_id/$unit_per_bank[$m_step-1])%4);
		my $min_bg_id=int(($min_unit_id/$unit_per_bank[$m_step-1])/4);
		my $max_bk_id=int(($min_unit_id/$unit_per_bank[$m_step-1])%4);
		my $max_bg_id=int(($min_unit_id/$unit_per_bank[$m_step-1])/4);
		
		print F_OUTPUT "$cols_to_open[$m_step-1] 1 $min_row_id $min_bk_id $min_bg_id $max_row_id $max_bk_id $max_bg_id $LINT_s[0]\n";#  $MIN_s[0]/$div  $MAX_s[0]/$div\n";
	}else{
		my $min_bk_id=int(($min_unit_id/2/$unit_per_bank[$m_step-1])%4);
		my $min_bg_id=int(($min_unit_id/2/$unit_per_bank[$m_step-1])/4)*2;
		my $max_bk_id=int(($min_unit_id/2/$unit_per_bank[$m_step-1])%4);
		my $max_bg_id=int(($min_unit_id/2/$unit_per_bank[$m_step-1])/4)*2;

		print F_OUTPUT "128 $rows_to_open[$m_step-1] $min_row_id $min_bk_id $min_bg_id $max_row_id $max_bk_id $max_bg_id $LINT_s[0]\n";#  $MIN_s[0]/$div  $MAX_s[0]/$div\n";
	}	
}

###for(my $i=0;$i<128;$i=$i+1){
###	print "$arr[$i]\n";
###}
###for(my $i=0;$i<8;$i=$i+1){
###	print "$rows_to_open[$i]\n";
###}
###
###for(my $i=0;$i<8;$i=$i+1){
###	print "$unit_num[$i]\n";
###}