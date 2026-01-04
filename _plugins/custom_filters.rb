module Jekyll
  module CustomFilters
    def chop_paragraph(input)
      input.slice!(input.index("<p>") + 3, input.rindex("</p>") - 3)
    end
  end
end

Liquid::Template.register_filter(Jekyll::CustomFilters)
