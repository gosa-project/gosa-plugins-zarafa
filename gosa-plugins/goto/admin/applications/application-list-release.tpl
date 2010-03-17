<table style='width:100%;height:100%; ' summary="">

  <tr>
    <td style='width:100%;'>

      <div class="contentboxh">
        <p class="contentboxh">&nbsp;{$HEADLINE}&nbsp;{$SIZELIMIT}</p>
      </div>
      
      <div class="contentboxb">
       <div style='background:white;padding:3px;'>
        <table><tr>
          <td>{$RELOAD}&nbsp;</td><td>{$SEPARATOR}&nbsp;</td>
          <td>{image path="images/rocket.png"}
</td>
          <td> {$ACTIONS}</td>
        </tr></table>
       </div>
      </div>
      
      <div style='height:4px;'>
      </div>
      {$LIST}
    </td>
    <td style='min-width:250px'>

      {$FILTER}
    </td>
  </tr>
</table>

<input type="hidden" name="ignore">
