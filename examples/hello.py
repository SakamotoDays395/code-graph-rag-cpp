# Example Python file for testing the parser
# Place test files here to verify your parser works correctly

def calculate_tax(amount, rate=0.18):
    """Calculate tax for a given amount."""
    return amount * rate


def calculate_total(subtotal):
    """Calculate total including tax."""
    tax = calculate_tax(subtotal)
    return subtotal + tax


class Invoice:
    """Represents an invoice."""

    def __init__(self, customer_name, items):
        self.customer_name = customer_name
        self.items = items

    def get_subtotal(self):
        return sum(item.price for item in self.items)

    def get_total(self):
        return calculate_total(self.get_subtotal())
