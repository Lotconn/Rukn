from flask_wtf import FlaskForm
from wtforms import StringField, SubmitField, TextAreaField, validators


class ContactForm(FlaskForm):
    name = StringField(
        "Name", validators=[validators.Length(max=64), validators.Optional()]
    )
    email = StringField(
        "Email",
        validators=[
            validators.Email(),
            validators.Optional(),
            validators.Length(max=64),
        ],
    )
    message = TextAreaField(
        "Message", validators=[validators.input_required(), validators.Length(max=1024)]
    )
    submit = SubmitField("Send")
