# SPDX-License-Identifier: MIT

class BaseValidator
  def initialize(check)
    @check = check
  end

  private

  def call(method_name, *args)
    @check.send(method_name, *args)
  end

  def fail!(message)
    @check.send(:fail!, message)
  end
end
