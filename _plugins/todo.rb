module Jekyll
  class TodoTagBlock < Liquid::Block
    def initialize(tag_name, arg, parse_context)
      super
      @arg = arg
    end
    def render(context)
      "<span style='text-color:red'>#{@arg}</span>"
    end
  end
end

Liquid::Template.register_tag('todo', Jekyll::TodoTagBlock)
