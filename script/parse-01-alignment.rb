#! /bin/env ruby

def run_fapppx(target_dir)
  result = {}
  outstr = `fapppx -A -I hwm -d #{target_dir} -t csv`
  outstr.each_line do |e|
    items = e.strip.split(',')
    next if items.size != 12
    if items[0] == '"Application"' && items[1] == '"AVG"'
      result['Elapsed(s)'] = items[2].sub(/"/, '').to_f
      result['MFLOPS'] = items[3].sub(/"/, '').to_f
      result['MIPS'] = items[5].sub(/"/, '').to_f
      result['Mem throughput_chip(GB/S)'] = items[7].sub(/"/, '').to_f
      break
    end
  end
  return result
end

def dir_size(target_dir)
  size = 0
  Dir.foreach(target_dir) do |e|
    next if /^\./ =~ e
    size += FileTest.size("#{target_dir}/#{e}")
  end
  return size
end


system('type fapppx 1>/dev/null')
if $?.exitstatus != 0
  $stderr.puts "ERROR: Fujitsu Profiler command 'fapppx' is not found."
  exit 1
end

if ARGV.size != 5
  $stderr.puts "Usage: #{$0} INPUT1 INPUT2 MED_DIR OUT_DIR PROF_DIR"
  exit 1
end

input1   = ARGV.shift
input2   = ARGV.shift
med_dir  = ARGV.shift
out_dir  = ARGV.shift
prof_dir = ARGV.shift

# parse Fujitsu Profiler reults
puts '-------------------'
puts 'Performance Profile'
puts '-------------------'
subdir = [ 'fapp_1_bwa', 'fapp_2_bwa', 'fapp_3_bwa',
           'fapp_4_splitSam2Contig2' ]
subdir.each do |e|
  res = run_fapppx("#{prof_dir}/#{e}")
  puts e
  puts "    Elapsed(s)                : #{res['Elapsed(s)']}"
  puts "    MFLOPS                    : #{res['MFLOPS']}"
  puts "    MIPS                      : #{res['MIPS']}"
  puts "    Mem throughput_chip(GB/S) : #{res['Mem throughput_chip(GB/S)']}"
end
puts

# check file size
size_input1 = FileTest.size(input1)
size_input2 = FileTest.size(input2)
size_input1_sai = FileTest.size("#{med_dir}/sra_1.fastq.0.sai")
size_input2_sai = FileTest.size("#{med_dir}/sra_2.fastq.0.sai")
size_algn_out   = FileTest.size("#{med_dir}/0.sam")
size_output = dir_size(out_dir)

puts '-----------'
puts 'Input Files'
puts '-----------'
puts 'fapp_1_bwa'
puts "    [TARGET] read 1        : #{size_input1}"
puts 'fapp_2_bwa'
puts "    [TARGET] read 2        : #{size_input2}"
puts 'fapp_3_bwa'
puts "    [TARGET] read 1        : #{size_input1}"
puts "    [TARGET] read 2        : #{size_input2}"
puts "    [MED] read 1 sa        : #{size_input1_sai}"
puts "    [MED] read 2 sa        : #{size_input2_sai}"
puts 'fapp_4_splitSam2Contig2'
puts "    [MED] alignment result : #{size_algn_out}"
puts

puts '------------'
puts 'Output Files'
puts '------------'
puts 'fapp_1_bwa'
puts "    [MED] read 1 sa               : #{size_input1_sai}"
puts 'fapp_2_bwa'
puts "    [MED] read 2 sa               : #{size_input2_sai}"
puts 'fapp_3_bwa'
puts "    [MED] alignment result        : #{size_algn_out}"
puts 'fapp_4_splitSam2Contig2'
puts "    [OUT] contig splitted results : #{size_output}"


# Local Variables:
# mode: Ruby
# coding: utf-8
# indent-tabs-mode: nil
# End:
