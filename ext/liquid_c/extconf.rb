require 'mkmf'
$CFLAGS << ' -std=c99 -Wall -Werror -Wextra -Wno-unused-parameter -Wno-missing-field-initializers'
compiler = RbConfig::MAKEFILE_CONFIG['CC']
if ENV['DEBUG'] == 'true'
  if compiler =~ /gcc|g\+\+/
    $CFLAGS << ' -fbounds-check'
  end
else
  $CFLAGS << ' -DNDEBUG'
end

if Gem::Version.new(RUBY_VERSION) >= Gem::Version.new("2.7.0") # added in 2.7
  $CFLAGS << ' -DHAVE_RB_HASH_BULK_INSERT'
end

$warnflags.gsub!(/-Wdeclaration-after-statement/, "") if $warnflags
create_makefile("liquid_c")
