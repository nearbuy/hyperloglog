require 'lib/hyperloglog'

describe 'A HyperBuilder' do
  before(:each) do
    @builder = HyperBuilder.new(11)
  end
  
  it 'should build bitsets' do
    0.upto(999) do |index|
      @builder.offer(index.to_s)
    end
    
    puts @builder.inspect
  end
  
  it 'should serialize' do
    0.upto(9) do |index|
      @builder.offer(index.to_s)
    end
    
    puts @builder.serialize.inspect
  end
end

describe 'A HyperEstimator with a small set' do
  before(:each) do
    builder = HyperBuilder.new(11)
    0.upto(9){ |index| builder.offer(index.to_s) }
    @estimator = HyperEstimator.new(11, builder.serialize)
  end
  
  it 'should generate great estimates for a single estimator' do
    HyperEstimator.estimate(@estimator)
  end
end