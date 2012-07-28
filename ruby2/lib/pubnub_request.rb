class PubnubRequest
  attr_accessor :callback, :operation, :callback, :publish_key, :subscribe_key, :secret_key, :channel, :jsonp, :message

  def initialize(args = {})
    args = HashWithIndifferentAccess.new(args)

    @operation = args[:operation].to_s
    @callback = args[:callback]
    @publish_key = args[:publish_key]
    @subscribe_key = args[:subscribe_key]
    @channel = args[:channel]
    @jsonp = args[:jsonp].present? ? "1" : "0"
    @message = args[:message]
    @secret_key = args[:secret_key] || "0"
  end

  def ==(another)
    self.operation == another.operation && self.callback == another.callback &&
      self.channel == another.channel && self.message == another.message
  end

end