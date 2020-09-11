require 'test_helper'

class BlockTest < MiniTest::Test
  def test_no_allocation_of_trimmed_strings
    template = Liquid::Template.parse("{{ a -}}     {{- b }}")
    assert_equal 2, template.root.nodelist.size

    template = Liquid::Template.parse("{{ a -}} foo {{- b }}")
    assert_equal 3, template.root.nodelist.size
  end

  def test_pre_trim
    template = Liquid::Template.parse("\n{%- raw %}{% endraw %}")
    assert_equal "", template.render

    template = Liquid::Template.parse("\n{%- if true %}{% endif %}")
    assert_equal "", template.render
  end

  # Temporary to test rollout of the fix for this bug
  def test_bug_compatible_pre_trim
    template = Liquid::Template.parse("\n {%- raw %}{% endraw %}", bug_compatible_whitespace_trimming: true)
    assert_equal "\n", template.render

    template = Liquid::Template.parse("\n {%- if true %}{% endif %}", bug_compatible_whitespace_trimming: true)
    assert_equal "\n", template.render

    template = Liquid::Template.parse("{{ 'B' }} \n{%- if true %}C{% endif %}", bug_compatible_whitespace_trimming: true)
    assert_equal "B C", template.render

    template = Liquid::Template.parse("B\n {%- raw %}{% endraw %}", bug_compatible_whitespace_trimming: true)
    assert_equal "B", template.render

    template = Liquid::Template.parse("B\n {%- if true %}{% endif %}", bug_compatible_whitespace_trimming: true)
    assert_equal "B", template.render
  end
end
