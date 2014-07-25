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

if ARGV.size != 4
  $stderr.puts "Usage: #{$0} INPUT MED_DIR OUT_DIR PROF_DIR"
  exit 1
end

input   = ARGV.shift
med_dir  = ARGV.shift
out_dir  = ARGV.shift
prof_dir = ARGV.shift

# parse Fujitsu Profiler reults
puts '-------------------'
puts 'Performance Profile'
puts '-------------------'
subdir = [ 'fapp_1_samtools', 'fapp_2_samtools', 'fapp_3_samtools' ]
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
size_input = FileTest.size(input)
file_prefix = input.split(/\//)[-1].sub(/\.sam$/, '')
size_bam = FileTest.size("#{med_dir}/#{file_prefix}.bam")
size_sort_bam = FileTest.size("#{med_dir}/#{file_prefix}.sort.bam")
size_rmdup_bam = FileTest.size("#{out_dir}/#{file_prefix}.bam")

puts '-----------'
puts 'Input Files'
puts '-----------'
puts 'fapp_1_samtools'
puts "    [TARGET] sam           : #{size_input}"
puts 'fapp_2_samtools'
puts "    [MED] bam              : #{size_bam}"
puts 'fapp_3_samtools'
puts "    [MED] sorted bam       : #{size_sort_bam}"
puts

puts '------------'
puts 'Output Files'
puts '------------'
puts 'fapp_1_samtools'
puts "    [MED] bam              : #{size_bam}"
puts 'fapp_2_samtools'
puts "    [MED] sorted bam       : #{size_sort_bam}"
puts 'fapp_3_samtools'
puts "    [OUT] rmdup bam        : #{size_rmdup_bam}"


# Local Variables:
# mode: Ruby
# coding: utf-8
# indent-tabs-mode: nil
# End:
